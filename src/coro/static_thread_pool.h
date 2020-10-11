#pragma once

#include <thread>
#include <deque>
#include <mutex>
#include <coroutine>
#include <vector>

#include "schedule.h"

namespace clu
{
    namespace detail
    {
        class task_queue final
        {
        private:
            std::mutex mutex_;
            std::condition_variable cv_;
            std::deque<std::coroutine_handle<>> queue_;
            bool stop_requested_ = false;

        public:
            void push(std::coroutine_handle<> coro);
            std::coroutine_handle<> pop();
            bool try_push(std::coroutine_handle<> coro);
            std::coroutine_handle<> try_pop();
            void request_stop();
        };
    }

    class static_thread_pool final
    {
    public:
        class awaiter final
        {
        private:
            static_thread_pool* pool_ = nullptr;

        public:
            explicit awaiter(static_thread_pool* pool): pool_(pool) {}

            bool await_ready() const noexcept { return false; }
            void await_suspend(const std::coroutine_handle<> handle) { pool_->enqueue_work(handle); }
            void await_resume() const noexcept {}
        };
        friend class awaiter;

    private:
        static constexpr size_t spin_times = 2;

        size_t thread_count_;
        std::atomic_size_t index_{ 0 };
        std::vector<std::thread> threads_;
        std::vector<detail::task_queue> queues_;

        void process(size_t index);
        void enqueue_work(std::coroutine_handle<> coro);

    public:
        explicit static_thread_pool(size_t thread_count = std::thread::hardware_concurrency());
        ~static_thread_pool() noexcept;

        awaiter schedule() { return awaiter(this); }
    };

    static_assert(scheduler<static_thread_pool>);
}
