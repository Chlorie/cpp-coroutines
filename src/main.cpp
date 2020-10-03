#include <chrono>
#include <iostream>

#include "task.h"
#include "spawn.h"
#include "static_thread_pool.h"

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

int main() // NOLINT
{
    clu::static_thread_pool pool;
    spawn([&]()-> clu::task<>
    {
        print_thread();
        for (size_t i = 0; i < 10; i++)
            spawn([&]()-> clu::task<>
            {
                co_await schedule_on(pool);
                print_thread();
            }());
        co_return;
    }());
}
