#include <array>
#include <chrono>
#include <cstring>
#include <memory>
#include <mutex>
#include <thread>
#include <thread>
#include <unordered_set>

#include "gtest/gtest.h"
#include "src/thread_pool.h"

// TODO(cbraley): These tests are pretty lame - I don't like using std::sleep
// in tests or relying timing - this makes tests prone to flakniess. This is
// good enough for now though...

namespace cb {

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
  std::mutex mu;
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
  std::cout << "Waiting on work to be done..." << std::endl;
  pool->Wait();
  std::cout << "Done waiting on work." << std::endl;
  {
    std::lock_guard<std::mutex> lk(mu);
    EXPECT_EQ(counter, kNumTasks);
  }
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
}

TEST_F(ThreadPoolTest, WaitWithWorkAlreadyDone) {
  constexpr int kNumTasks = 64;
  std::mutex mu;
  int counter = 0;
  std::unique_ptr<ThreadPool> pool = MakePool();
  EXPECT_EQ(pool->OutstandingWorkSize(), 0);
  for (int i = 0; i < kNumTasks; ++i) {
    pool->Schedule([&]() {
      std::lock_guard<std::mutex> lk(mu);
      counter++;
    });
  }
  std::this_thread::sleep_for(std::chrono::seconds(10));
  std::cout << "Waiting on work to be done..." << std::endl;
  pool->Wait();
  std::cout << "Done waiting on work." << std::endl;
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

}  // namespace cb
