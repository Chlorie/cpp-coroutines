# C++ Coroutines

Test things out and try to learn C++20 coroutines.

- `concepts.h`: Basic coroutine concepts.

  ```cpp
  static_assert(clu::awaiter<clu::task<>>);
  static_assert(clu::awaiter_of<clu::task<int>, int>);
  static_assert(std::same_as<clu::awaiter_result_t<clu::task<int>&&>, int&&>);
  ```
  
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
  
- `schedule.h`: Defines `scheduler` concept and provides a generic `schedule_on` function template for scheduling tasks.

  ```cpp
  static_assert(clu::scheduler<clu::static_thread_pool>);
  
  clu::task<> scheduled_task(clu::scheduler auto& sch)
  {
      co_await clu::schedule_on(sch);
      std::cout << "On scheduler!\n";
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

- `static_thread_pool.h / .cpp`: Provides a simple static thread pool implementation `static_thread_pool`. The thread pool type satisfies `scheduler`.

  ```cpp
  clu::static_thread_pool pool;
  
  clu::task<> task()
  {
      std::cout << "On original thread!\n";
      co_await pool.schedule();
      std::cout << "Now I'm on the thread pool!\n";
  }
  ```
  
- `sync_wait.h`: Wait for an awaitable's completion on the current thread and retrieve the result using `sync_wait`.

  ```cpp
  clu::task<> task()
  {
      std::this_thread::sleep_for(1s);
      co_return 42;
  }
  
  int main()
  {
      const int answer = sync_wait(task());
      std::cout << "The answer is " << answer << '\n';
  }
  ```
  
- `task.h`: Lazy task type `task<T>`, supports `co_await` operations.

  ```cpp
  clu::task<> some_task()
  {
      const int result = co_await async_long_task();
      co_return 42 + result;
  }
  ```
