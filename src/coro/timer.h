#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

#include "schedule.h"
#include "task.h"

namespace clu
{
    namespace detail
    {
        using timer_clock = std::chrono::steady_clock;

        struct timer_work final
        {
            timer_clock::time_point time{};
            std::coroutine_handle<> task{};

            friend bool operator<(const timer_work lhs, const timer_work rhs)
            {
                return lhs.time < rhs.time;
            }

            bool is_due() const noexcept { return time <= timer_clock::now(); }
        };
    }

    class timer final
    {
    private:
        using clock = detail::timer_clock;

        class awaiter final
        {
        private:
            timer* timer_ = nullptr;
            clock::time_point time_;

        public:
            explicit awaiter(timer* timer, const clock::time_point time): timer_(timer), time_(time) {}

            bool await_ready() const noexcept { return false; }
            void await_suspend(const std::coroutine_handle<> handle) const { timer_->enqueue_work({ time_, handle }); }
            void await_resume() const noexcept {}
        };
        friend class awaiter;

        bool stop_requested_ = false;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::priority_queue<detail::timer_work> queue_;
        std::thread thread_;

        template <duration Dur>
        static auto as_clock_duration(const Dur dur) { return std::chrono::duration_cast<clock::duration>(dur); }

        template <duration Dur>
        static auto time_after(const Dur delay) { return clock::now() + as_clock_duration(delay); }

        void enqueue_work(detail::timer_work work);
        void process();
        void request_stop();

    public:
        timer();
        ~timer() noexcept;

        awaiter wait_until(const clock::time_point time) { return awaiter(this, time); }

        template <scheduler Sch>
        task<> schedule_on_at(Sch& sch, const clock::time_point time)
        {
            co_await wait_until(time);
            co_await sch.schedule();
        }

        template <duration Dur>
        auto wait_for(const Dur delay) { return wait_until(time_after(delay)); }

        template <scheduler Sch, duration Dur>
        task<> schedule_on_after(Sch& scheduler, const Dur delay) { return schedule_on_at(scheduler, time_after(delay)); }

        template <std::invocable Func, duration Per, duration Init = std::chrono::seconds>
        task<> call_every(Func func, Per period, Init initial_delay = std::chrono::seconds(0))
        {
            // Parameters are not const to avoid C4269 (workaround 16.8 pre3)
            const clock::time_point initial = time_after(initial_delay);
            co_await wait_until(initial);
            for (size_t i = 1;; i++)
            {
                func();
                co_await wait_until(initial + as_clock_duration(i * period));
            }
        }
    };
}
