#pragma once

#include <coroutine>

#include "schedule.h"

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

    template <awaitable_of<void> A> detail::spawn_t spawn(A awt) { co_await awt; }

    template <awaitable_of<void> A, scheduler S>
    detail::spawn_t spawn_on(A awt, S& sch)
    {
        co_await sch.schedule();
        co_await awt;
    }
}
