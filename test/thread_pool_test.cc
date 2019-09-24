#include <array>
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <unordered_set>

#include "gtest/gtest.h"
#include "src/thread_pool.h"

namespace cb {

// Simple counting semaphore class.
class Semaphore {
 public:
  Semaphore() : sema_count_(0) {}

  void Notify() {
    std::unique_lock<std::mutex> lock(mu_);
    sema_count_++;
    condvar_.notify_one();
  }

  void Wait() {
    std::unique_lock<std::mutex> lock(mu_);
    while (sema_count_ == 0) {
      condvar_.wait(lock);
    }
    sema_count_--;
  }

 private:
  std::mutex mu_;
  std::condition_variable condvar_;
  int sema_count_;
};

class ThreadPoolTest : public ::testing::Test {
 protected:
  ThreadPoolTest() {}
  std::unique_ptr<ThreadPool> MakePool() const {
    constexpr int kPoolSizeForTest = 4;
    std::unique_ptr<ThreadPool> pool(new ThreadPool(kPoolSizeForTest));
    std::cout << "Num workers = " << pool->NumWorkers() << "\n";
    return pool;
  }

  ~ThreadPoolTest() override {}
};

TEST_F(ThreadPoolTest, BasicSanity) {
  std::mutex stdout_mu;
  std::unique_ptr<ThreadPool> pool = MakePool();
  for (int i = 0; i < 10; ++i) {
    pool->Schedule([&stdout_mu, i]() {
      std::this_thread::sleep_for(std::chrono::seconds(i));
      std::lock_guard<std::mutex> stdout_lock_guard(stdout_mu);
      std::cout << "Hello from thread " << std::this_thread::get_id()
                << std::endl;
    });
  }
}

TEST_F(ThreadPoolTest, Wait) {
  constexpr int kNumTasks = 64;
  std::mutex mu;  // Guard counter.
  int counter = 0;

  std::unique_ptr<ThreadPool> pool = MakePool();

  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
  for (int i = 0; i < kNumTasks; ++i) {
    pool->Schedule([&]() {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      std::lock_guard<std::mutex> lk(mu);
      counter++;
    });
    EXPECT_LE(pool->OutstandingWorkSize(), i + 1);
  }

  {
    std::lock_guard<std::mutex> lk(mu);
    EXPECT_LE(counter, kNumTasks);
    EXPECT_GE(counter, 0);
  }
  pool->Wait();
  {
    std::lock_guard<std::mutex> lk(mu);
    EXPECT_EQ(counter, kNumTasks);
  }
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
}

TEST_F(ThreadPoolTest, WaitWithWorkAlreadyDone) {
  constexpr int kNumTasks = 64;
  std::mutex mu;  // Guard counter.
  int counter = 0;
  std::unique_ptr<ThreadPool> pool = MakePool();
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
  for (int i = 0; i < kNumTasks; ++i) {
    pool->Schedule([&]() {
      std::lock_guard<std::mutex> lk(mu);
      counter++;
    });
  }

  // Hackily wait until all threads are done.
  while (counter < kNumTasks) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  pool->Wait();
  {
    std::lock_guard<std::mutex> lk(mu);
    EXPECT_EQ(counter, kNumTasks);
  }
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
}

TEST_F(ThreadPoolTest, WaitCalledWithNoWorkDoesNotDeadlock) {
  std::unique_ptr<ThreadPool> pool = MakePool();
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
  pool->Wait();
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
}

TEST_F(ThreadPoolTest, NumUniqueWorkerIds) {
  std::mutex mu;
  std::unordered_set<std::thread::id> tids_seen;

  constexpr int kNumWorkers = 12;
  {
    std::unique_ptr<ThreadPool> pool(new ThreadPool(kNumWorkers));
    EXPECT_EQ(pool->OutstandingWorkSize(), 0);
    for (int i = 0; i < kNumWorkers * 3; ++i) {
      pool->Schedule([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        const std::thread::id tid = std::this_thread::get_id();
        std::lock_guard<std::mutex> lk(mu);
        tids_seen.insert(tid);
      });
    }
  }

  EXPECT_LE(tids_seen.size(), kNumWorkers);
}

TEST_F(ThreadPoolTest, FuturesThatReturnVoid) {
  Semaphore sema;

  std::unique_ptr<ThreadPool> pool = MakePool();
  std::future<void> future =
      pool->ScheduleAndGetFuture([&sema]() { sema.Wait(); });

  EXPECT_NE(future.wait_for(std::chrono::seconds(1)),
            std::future_status::ready);
  sema.Notify();
  future.wait();
}

int Sum(int x, int y) { return x + y; }

TEST_F(ThreadPoolTest, FuturesThatReturnNonVoid) {
  Semaphore sema;

  std::unique_ptr<ThreadPool> pool = MakePool();
  std::future<int> future = pool->ScheduleAndGetFuture([&sema]() {
    sema.Wait();
    return Sum(1, 99);
  });

  EXPECT_NE(std::future_status::ready,
            future.wait_for(std::chrono::seconds(5)));
  sema.Notify();
  future.wait();
  EXPECT_EQ(future.get(), 100);
}

void PrintSum(int x, int y) {
  std::cout << "The sum is " << x + y << std::endl;
}

TEST_F(ThreadPoolTest, VoidFuture) {
  Semaphore sema;
  std::unique_ptr<ThreadPool> pool = MakePool();
  std::future<void> future = pool->ScheduleAndGetFuture([&sema]() {
    sema.Wait();
    PrintSum(1, 99);
  });

  EXPECT_NE(std::future_status::ready,
            future.wait_for(std::chrono::seconds(5)));
  sema.Notify();
  future.wait();
}

TEST_F(ThreadPoolTest, ForwardingArguments) {
  std::unique_ptr<ThreadPool> pool = MakePool();
  std::future<int> sum_future = pool->ScheduleAndGetFuture(&Sum, 3, 1);
  EXPECT_EQ(sum_future.get(), 4);
}

class ClassWithAMemberFunction {
 public:
  int x = 0;
  int AddX(int input) const { return x + input; }
};

// TODO(cbraley): This test won't pass unless we can use std::invoke from C++17. 
// Consider finding a workaround for C++11.
#if __cplusplus >= 201703L
TEST_F(ThreadPoolTest, InvokingMemberFunctions) {
  std::unique_ptr<ThreadPool> pool = MakePool();
  ClassWithAMemberFunction object;
  object.x = 12;
  std::future<int> sum_future =
      pool->ScheduleAndGetFuture(&ClassWithAMemberFunction::AddX, &object, 3);
  EXPECT_EQ(sum_future.get(), 15);
}
#endif

}  // namespace cb
