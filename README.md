# C++ Thread Pool
Simple C++ thread pool class with no external dependencies. This class can be 
built with any C++ version >= C++11. This thread pool is implemented using a 
single work queue, and a fixed size pool of worker threads. Work 
items(functions) are processed in a FIFO order.

## Why is this useful? Why not use [`std::async`](https://en.cppreference.com/w/cpp/thread/async)?
Before writing this code, I tried using `std::async`. However, I ran into the
following problems:
1. `std::async(std::launch::async, ...)` launches a new thread for every 
    invocation. On Mac and Linux, no thread pool is used so you have to 
    pay the price of thread creation (about 0.5ms on my laptop) for each 
    new call.
1. If you use `std::async` you must have to carefully manage the number of 
   in-flight `std::futures` to achieve peak performance.
1. Any `thread_local` variables must also be destroyed each time a thread is 
   shutdown. This is inefficient, since `std::async` launches a new thread
   per call. A thread pool, however, only pays this destruction cost once when
   the pool is destroyed.

## Good Workloads
This simple design works well for embarrassingly parallel workloads that don't 
block for long periods of time. Many graphics, image processing, and compute 
vision applications meet this requirement. In this case, you want to set the 
thread pool size to `ThreadPool::GetDefaultThreadPoolSize()` which typically 
returns the number of logical cores your machine has.

## Bad Workloads
Workloads that block for long durations(disk IO, holding locks for a long time, 
etc.) won't perform well with this thread pool design, especially if you set 
your thread pool size to the number of logical cores. Very short tasks (a few ms)
also aren't a good fit. As a workaround, you can simply set the number of 
threads to a very large number (N * number of logical cores). However, this 
solution is still suboptimal. A better approach is to use a thread pool that 
implements work stealing. This is because the blocked function will occupy one 
of the threads in the pool, and we    

## Build instructions

* Install [bazel](https://bazel.build/)
* Run unit tests and/or benchmarks if you desire:

```shell
./scripts/build_and_run_unit_tests.sh
./scripts/build_and_run_unit_benchmarks.sh
```

* Write your own code that uses `src/thread_pool.h`. If you are using 
  [bazel](https://bazel.build/), this is as easy as depending on
  `src:thread_pool`.

## Outstanding Issues

1. TODO: Writeup info on the issues with `std::async`, and why one might want to
   use this repo instead.

1. Writeup benchmark info.

1. Include [thread sanitizer](https://clang.llvm.org/docs/ThreadSanitizer.html) tests.

1. Add [thread safety annotations](https://clang.llvm.org/docs/ThreadSafetyAnalysis.html).

1. Ideally, the `ScheduleAndGetFuture` function would be able to be called with 
  the exact same arguments you would pass to
  [`std::async`](https://en.cppreference.com/w/cpp/thread/async). This is
  true if you are compiling with C++17, but with earlier C++ versions there is 
  a slight limitation. Without C++17, you can't "directly" invoke member 
  functions as you would with `std::async`. For example, the following code is 
  valid C++11:

  ```cpp
  class MyClass {
   public:
     explicit MyClass(int value) : value_(value) {}

     int ComputeSum(int a) const {
       return a + value_;
     }
   private:
    int value_;
  };

  int main() {
   MyClass object(12);
   std::future<int> the_sum = std::async(std:launch::async,
                                         &MyClass::ComputeSum, 8);
   std::cout << the_sum.get() << std::endl;  // Prints 20.
  }
  ```

  However, a similar piece of code using `thread_pool.h` won't compile with 
  C++11...but it does work with C++17. 


  ```cpp
  int main() {
    MyClass object(12);
    ThreadPool pool(4);
    // The line below will compile with C++17, but will fail to compile on older 
    // C++ versions.
    std::future<int> the_sum = pool.ScheduleAndGetFuture(&MyClass::ComputeSum, 8);
    std::cout << the_sum.get() << std::endl;  // Prints 20.
  }
  ```

  You can use a simple lambda to workaround this issue if you can't use c++17:

  ```cpp
  int main() {
    MyClass object(12);
    ThreadPool pool(4);
    std::future<int> the_sum = pool.ScheduleAndGetFuture([&object](int a) {
      return object.ComputeSum(a);
    }, 8);
    std::cout << the_sum.get() << std::endl;  // Prints 20.
  }
  ```

  The root of this issue is that 
  [`std::invoke`](https://en.cppreference.com/w/cpp/utility/functional/invoke) 
  isn't available before C++17. Without `std::invoke` calling a member function
  in a generic way requires surprisingly tricky template metaprogramming.

1. Include more example code.

1. Remove `ThreadPool::Schedule` and use `ThreadPool::ScheduleAndGetFuture`
   everywhere. Users can just ignore the returned `std::future` if needed.

1. If we ever want to handle tasks that block for long periods, we should
   investigate work stealing and using 1 queue per thread. The latter change is 
   invasive, since we need to know when a thread is blocked for a "long enough"
   time.

1. Handle exceptions properly.

