# threadpool
Simple C++thread pool class with no external dependencies. This class can be 
built with any C++ version >= C++11. 

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

