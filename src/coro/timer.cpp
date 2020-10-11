#include "timer.h"

namespace clu
{
    void timer::enqueue_work(const detail::timer_work work)
    {
        if ([&]()
        {
            std::scoped_lock lock(mutex_);
            const bool result = queue_.empty() ? true : work < queue_.top();
            queue_.emplace(work);
            return result;
        }())
            cv_.notify_one();
    }

    void timer::process()
    {
        std::unique_lock lock(mutex_);
        while (!stop_requested_)
        {
            if (queue_.empty()) cv_.wait(lock);
            if (stop_requested_) return;
            while (!queue_.top().is_due())
            {
                cv_.wait_until(lock, queue_.top().time);
                if (stop_requested_) return;
            }
            while (!queue_.empty() && queue_.top().is_due())
            {
                const auto top = queue_.top();
                queue_.pop();
                lock.unlock();
                top.task();
                lock.lock();
            }
        }
    }

    void timer::request_stop()
    {
        {
            std::scoped_lock lock(mutex_);
            stop_requested_ = true;
        }
        cv_.notify_one();
    }

    timer::timer(): thread_(&timer::process, this) {}

    timer::~timer() noexcept
    {
        try
        {
            request_stop();
            thread_.join();
        }
        catch (...) { std::terminate(); }
    }
}
