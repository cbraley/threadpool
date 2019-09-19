#include "src/thread_pool.h"

#include <cassert>

namespace cb {

namespace {

// RAII helper class. `func` will be invoked upon destruction.
class Cleanup {
public:
  Cleanup(std::function<void(void)> func) : func_(std::move(func)) {}
  ~Cleanup() { func_(); }

private:
  std::function<void(void)> func_;
};

} // namespace

// static
int ThreadPool::GetDefaultThreadPoolSize() {
  const int dflt = std::thread::hardware_concurrency();
  assert(dflt >= 0);
  if (dflt == 0) {
    return 16;
  } else {
    return dflt;
  }
}

ThreadPool::~ThreadPool() {
  // TODO(cbraley): The current thread could help out to drain the work_ queue
  // faster - for example, if there is work that hasn't yet been scheduled this
  // thread could "pitch in" to help finish faster.

  {
    std::lock_guard<std::mutex> scoped_lock(mu_);
    exit_ = true;
  }
  condvar_.notify_all(); // Tell *all* workers we are ready.

  for (std::thread &thread : workers_) {
    thread.join();
  }
}

void ThreadPool::Wait() {
  std::unique_lock<std::mutex> lock(mu_);
  if (!work_.empty()) {
    work_done_condvar_.wait(lock, [this] { return work_.empty(); });
  }
}

ThreadPool::ThreadPool(int num_workers)
    : num_workers_(num_workers), exit_(false) {
  assert(num_workers_ > 0);
  workers_.reserve(num_workers_);
  for (int i = 0; i < num_workers_; ++i) {
    workers_.emplace_back(&ThreadPool::ThreadLoop, this);
  }
}

void ThreadPool::Schedule(std::function<void(void)> func) {
  {
    std::lock_guard<std::mutex> scoped_lock(mu_);
    work_.emplace(std::move(func));
  }
  condvar_.notify_one(); // Tell one worker we are ready.
}

void ThreadPool::ThreadLoop() {
  // Wait until the ThreadPool sends us work.
  while (true) {
    std::function<void(void)> work_func;

    int prev_work_size = -1;
    {
      std::unique_lock<std::mutex> lock(mu_);
      condvar_.wait(lock, [this] { return exit_ || (!work_.empty()); });
      // ...after the wait(), we hold the lock.

      // If all the work is done and exit_ is true, break out of the loop.
      if (exit_ && work_.empty()) {
        break;
      }

      // Pop the work off of the queue - we are careful to execute the
      // work_func() callback only after we have released the lock.
      work_func = std::move(work_.front());
      prev_work_size = work_.size();
      work_.pop();
    }

    // We are careful to do the work without the lock held!
    work_func(); // Do work.

    // Notify a condvar is all work is done.
    {
      std::unique_lock<std::mutex> lock(mu_);
      if (work_.empty() && prev_work_size == 1) {
        work_done_condvar_.notify_all();
      }
    }
  }
}

int ThreadPool::OutstandingWorkSize() const {
  std::lock_guard<std::mutex> scoped_lock(mu_);
  return work_.size();
}

int ThreadPool::NumWorkers() const { return num_workers_; }

} // namespace cb
