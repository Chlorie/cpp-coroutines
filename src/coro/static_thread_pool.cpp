#include "static_thread_pool.h"

namespace clu
{
    namespace detail
    {
        void task_queue::push(const std::coroutine_handle<> coro)
        {
            {
                std::unique_lock lock(mutex_);
                queue_.push_back(coro);
            }
            cv_.notify_one();
        }

        std::coroutine_handle<> task_queue::pop()
        {
            std::unique_lock lock(mutex_);
            while (queue_.empty() && !stop_requested_) cv_.wait(lock);
            if (queue_.empty()) return {};
            const std::coroutine_handle<> coro = queue_.front();
            queue_.pop_front();
            return coro;
        }

        bool task_queue::try_push(const std::coroutine_handle<> coro)
        {
            {
                const std::unique_lock lock(mutex_, std::try_to_lock);
                if (!lock) return false;
                queue_.push_back(coro);
            }
            cv_.notify_one();
            return true;
        }

        std::coroutine_handle<> task_queue::try_pop()
        {
            const std::unique_lock lock(mutex_, std::try_to_lock);
            if (!lock || queue_.empty()) return {};
            const std::coroutine_handle<> coro = queue_.front();
            queue_.pop_front();
            return coro;
        }

        void task_queue::request_stop()
        {
            {
                std::scoped_lock lock(mutex_);
                stop_requested_ = true;
            }
            cv_.notify_all();
        }
    }

    void static_thread_pool::process(const size_t index)
    {
        const auto loop = [&]()
        {
            for (size_t i = 0; i < spin_times * thread_count_; i++)
                if (const auto coro = queues_[(index + i) % thread_count_].try_pop())
                {
                    coro.resume();
                    return true;
                }
            const auto coro = queues_[index].pop();
            if (coro) coro.resume();
            return static_cast<bool>(coro);
        };
        while (loop());
    }

    void static_thread_pool::enqueue_work(const std::coroutine_handle<> coro)
    {
        const size_t index = index_.fetch_add(1, std::memory_order_relaxed);
        for (size_t i = 0; i < spin_times * thread_count_; i++)
            if (queues_[(index + i) % thread_count_].try_push(coro))
                return;
        queues_[index & thread_count_].push(coro);
    }

    static_thread_pool::static_thread_pool(const size_t thread_count):
        thread_count_(thread_count), queues_(thread_count)
    {
        for (size_t i = 0; i < thread_count; i++)
            threads_.emplace_back(&static_thread_pool::process, this, i);
    }

    static_thread_pool::~static_thread_pool() noexcept
    {
        try
        {
            for (auto& queue : queues_) queue.request_stop();
            for (auto& thread : threads_) thread.join();
        }
        catch (...) { std::terminate(); }
    }
}
