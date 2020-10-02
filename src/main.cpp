#include <chrono>
#include <iostream>

#include "task.h"
#include "spawn.h"
#include "generator.h"

using namespace std::literals;

template <typename F>
void time_call(F&& func)
{
    const auto start = std::chrono::high_resolution_clock::now();
    std::forward<F>(func)();
    const auto end = std::chrono::high_resolution_clock::now();
    std::cout << "Elapsed " << (end - start) / 1.0s << "s\n";
}

clu::generator<int> fib()
{
    int a = 1, b = 0;
    while (true)
    {
        a = std::exchange(b, a + b);
        co_yield b;
    }
}

int main() // NOLINT
{
    for (auto g = fib(); 
        const int i : g | std::views::take(10))
        std::cout << i << ' ';
}
