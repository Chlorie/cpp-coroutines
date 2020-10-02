# C++ Coroutines

Test things out and try to learn C++20 coroutines.

- `generator.h`: Generator type `generator<T>`, supports `co_yield` operations.

  ```cpp
  clu::generator<int> fib()
  {
      int a = 1, b = 0;
      while (true)
      {
          a = std::exchange(b, a + b);
          co_yield b;
      }
  }
  static_assert(std::ranges::input_range<clu::generator<int>>);
  ```

- `task.h`: Lazy task type `task<T>`, supports `co_await` operations.

  ```cpp
  clu::task<> some_task()
  {
      const int result = co_await async_long_task();
      co_return 42 + result;
  }
  ```

- `spawn.h`: Spawn a detached coroutine with `spawn(awaitable)`.

  ```cpp
  void non_coroutine()
  {
      spawn(some_task()); // Be careful that the task is detached
      do_other_things();
  }
  ```
