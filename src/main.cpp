#include <fmt/core.h>
#include <fmt/ostream.h>

#include "coro/spawn.h"
#include "coro/static_thread_pool.h"
#include "coro/task.h"
#include "coro/sync_wait.h"
#include "coro/timer.h"
#include "coro/generator.h"

using namespace std::literals;

template <std::invocable F>
void time_call(F&& func)
{
    const auto start = std::chrono::high_resolution_clock::now();
    std::invoke(std::forward<F>(func));
    const auto end = std::chrono::high_resolution_clock::now();
    fmt::print("Elapsed {}s\n", (end - start) / 1.0s);
}

std::mutex cout_mutex;

void print_thread()
{
    std::scoped_lock lock(cout_mutex);
    fmt::print("Now on thread {}\n", std::this_thread::get_id());
}

clu::static_thread_pool pool(7);
clu::timer timer;

void thread_switching_test() // TODO: next target: when_all
{
    sync_wait([&]()-> clu::task<>
    {
        print_thread();
        for (size_t i = 0; i < 4; i++)
            spawn_on([&]()-> clu::task<>
            {
                print_thread();
                std::this_thread::sleep_for(1s);
                co_await schedule_on(pool);
                print_thread();
            }(), pool);
        co_return;
    }());
}

clu::task<> timer_test()
{
    for (size_t i = 0; i < 10; i++)
    {
        co_await timer.schedule_on_after(pool, 1s);
        fmt::print("i = {}, thread id = {}\n", i, std::this_thread::get_id());
    }
}

clu::generator<int> gen()
{
    for (int i = 0;; i++)
    {
        fmt::print("i = {}\n", i);
        co_yield i;
    }
}

int main() // NOLINT
{
    spawn(timer.call_every(gen(), 1s, 2s));
    std::getchar();
    return 0;
}
