#pragma once

#include <coroutine>

namespace clu
{
    namespace detail
    {
        struct spawn_t final
        {
            struct promise_type final
            {
                spawn_t get_return_object() const noexcept { return {}; }
                std::suspend_never initial_suspend() const noexcept { return {}; }
                std::suspend_never final_suspend() const noexcept { return {}; }
                void return_void() const noexcept {}
                void unhandled_exception() const noexcept(false) { std::terminate(); }
            };
        };
    }

    template <typename Awaitable>
    detail::spawn_t spawn(Awaitable awaitable) { co_await awaitable; }
}
