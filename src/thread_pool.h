#ifndef SRC_THREAD_POOL_H_
#define SRC_THREAD_POOL_H_

// A simple thread pool class.
// Usage example:
//
// {
//   ThreadPool pool(16);  // 16 worker threads.
//   for (int i = 0; i < 100; ++i) {
//     pool.Schedule([i]() {
//       DoSlowExpensiveOperation(i);
//     });
//   }
//
//   // `pool` goes out of scope here - the code will block in the ~ThreadPool
//   // destructor until all work is complete.
// }
//

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace cb {

class ThreadPool {
 public:
  // Create a thread pool with `num_workers` dedicated worker threads.
  explicit ThreadPool(int num_workers);

  // Default construction is disallowed.
  ThreadPool() = delete;

  // Get the default thread pool size. This is implemented using
  // std::thread::hardware_concurrency().
  // https://en.cppreference.com/w/cpp/thread/thread/hardware_concurrency
  // On my machine this returns the number of logical cores.
  static int GetDefaultThreadPoolSize();

  // The `ThreadPool` destructor blocks until all outstanding work is complete.
  ~ThreadPool();

  // No copying, assigning, or std::move-ing.
  ThreadPool& operator=(const ThreadPool&) = delete;
  ThreadPool(const ThreadPool&) = delete;
  ThreadPool(ThreadPool&&) = delete;
  ThreadPool& operator=(ThreadPool&&) = delete;

  // Add the function `func` to the thread pool. `func` will be executed at some
  // point in the future on an arbitrary thread.
  void Schedule(std::function<void(void)> func);

  // Wait for all outstanding work to be completed.
  void Wait();

  // Return the number of outstanding functions to be executed.
  int OutstandingWorkSize() const;

  // Return the number of threads in the pool.
  int NumWorkers() const;

 private:
  void ThreadLoop();

  // Number of worker threads - fixed at construction time.
  int num_workers_;

  // The destructor sets `exit_` to true and then notifues all workers. `exit_`
  // causes each thread to break out of their work loop.
  bool exit_;

  mutable std::mutex mu_;

  // Queue of work. Guarded by `mu_`.
  std::queue<std::function<void(void)>> work_;

  // Condition variable used to notify worker threads that new work is
  // available.
  std::condition_variable condvar_;

  // Worker threads.
  std::vector<std::thread> workers_;

  // Condition variable used to notify that all work is complete - the work
  // queue has "run dry".
  std::condition_variable work_done_condvar_;
};

}  // namespace cb

#endif  // SRC_THREAD_POOL_H_
