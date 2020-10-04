#include <chrono>
#include <iostream>

#include "coro/spawn.h"
#include "coro/static_thread_pool.h"
#include "coro/task.h"
#include "coro/sync_wait.h"

using namespace std::literals;

template <std::invocable F>
void time_call(F&& func)
{
    const auto start = std::chrono::high_resolution_clock::now();
    std::invoke(std::forward<F>(func));
    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed " << (end - start) / 1.0s << "s\n";
}

std::mutex cout_mutex;

void print_thread()
{
    std::scoped_lock lock(cout_mutex);
    std::cout << "Now on thread " << std::this_thread::get_id() << '\n';
}

clu::static_thread_pool pool;

int main() // NOLINT
{
    sync_wait([&]()-> clu::task<>
    {
        print_thread();
        for (size_t i = 0; i < 4; i++)
            spawn_on([&]()-> clu::task<> // TODO: next target: when_all
            {
                print_thread();
                std::this_thread::sleep_for(1s);
                co_await schedule_on(pool);
                print_thread();
            }(), pool);
        co_return;
    }());
}
