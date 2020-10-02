#pragma once

#include <coroutine>
#include <variant>
#include <concepts>

namespace clu
{
    template <typename T> class task;

    namespace detail
    {
        template <typename T>
        class task_promise_base
        {
        private:
            struct final_awaitable final
            {
                bool await_ready() const noexcept { return false; }
                template <typename P>
                std::coroutine_handle<> await_suspend(const std::coroutine_handle<P> handle) const { return handle.promise().cont_; }
                void await_resume() const noexcept {}
            };
            std::coroutine_handle<> cont_{};

        public:
            std::suspend_always initial_suspend() const noexcept { return {}; }
            final_awaitable final_suspend() const noexcept { return {}; }

            void set_continuation(const std::coroutine_handle<> cont) noexcept { cont_ = cont; }
        };

        template <typename T>
        class task_promise final : public task_promise_base<T>
        {
        private:
            std::variant<std::monostate, T, std::exception_ptr> result_;

            template <typename U>
            static decltype(auto) get_impl(U&& self)
            {
                if (std::exception_ptr* eptrptr = std::get_if<2>(&self.result_))
                    std::rethrow_exception(*eptrptr);
                return std::get<1>(std::forward<U>(self).result_);
            }

        public:
            task<T> get_return_object();

            template <typename U> requires std::convertible_to<U&&, T>
            void return_value(U&& value) noexcept { result_.template emplace<1>(std::forward<U>(value)); }
            void unhandled_exception() noexcept { result_.template emplace<2>(std::current_exception()); }

            T& get() & { return get_impl(*this); }
            const T& get() const & { return get_impl(*this); }
            T&& get() && { return get_impl(std::move(*this)); }
            const T&& get() const && { return get_impl(std::move(*this)); }
        };

        template <>
        class task_promise<void> final : public task_promise_base<void>
        {
        private:
            std::exception_ptr eptr_;

        public:
            task<void> get_return_object();
            void return_void() const noexcept {}
            void unhandled_exception() noexcept { eptr_ = std::current_exception(); }

            void get() const { if (eptr_) std::rethrow_exception(eptr_); }
        };

        template <typename T>
        class task_promise<T&> final : public task_promise_base<T&>
        {
        private:
            T* ptr_ = nullptr;
            std::exception_ptr eptr_;

        public:
            task<T&> get_return_object();
            void return_value(T& value) noexcept { ptr_ = std::addressof(value); }
            void unhandled_exception() noexcept { eptr_ = std::current_exception(); }

            T& get() const
            {
                if (eptr_)
                    std::rethrow_exception(eptr_);
                return *ptr_;
            }
        };
    }

    template <typename T = void>
    class task final
    {
    public:
        using promise_type = detail::task_promise<T>;

    private:
        using handle_t = std::coroutine_handle<promise_type>;
        handle_t handle_{};

    public:
        explicit task(promise_type& promise): handle_(handle_t::from_promise(promise)) {}
        task(const task&) = delete;
        task(task&& other) noexcept: handle_(std::exchange(other.handle_, {})) {}
        task& operator=(const task&) = delete;
        task& operator=(task&& other) noexcept
        {
            if (this == &other) return *this;
            if (handle_) handle_.destroy();
            handle_ = std::exchange(other.handle_, {});
            return *this;
        }
        ~task() noexcept { if (handle_) handle_.destroy(); }

        bool await_ready() const noexcept { return false; }

        std::coroutine_handle<> await_suspend(const std::coroutine_handle<> handle)
        {
            handle_.promise().set_continuation(handle);
            return handle_;
        }

        decltype(auto) await_resume() const & { return handle_.promise().get(); }
        decltype(auto) await_resume() const && { return std::move(handle_.promise()).get(); }
    };

    namespace detail
    {
        template <typename T> task<T> task_promise<T>::get_return_object() { return task<T>(*this); }
        inline task<> task_promise<void>::get_return_object() { return task<void>(*this); }
        template <typename T> task<T&> task_promise<T&>::get_return_object() { return task<T&>(*this); }
    }
}
