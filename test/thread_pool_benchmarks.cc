#include <benchmark/benchmark.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <iostream>

#include "src/thread_pool.h"

// Default number of iterations when simulating a CPU bound task.
constexpr uint64_t kNumIterations = 50000;

// When comparing std::async to launching using a thread pool, we spawn this
// many tasks per benchmark iteration.
constexpr int kNumTasks = 10000;

// Synthetic CPU bound task that applies std::cos repeatedly.
// This computes http://mathworld.wolfram.com/DottieNumber.html
void CpuTask(uint64_t n = kNumIterations) {
  constexpr double kStartValue = 1.24;
  double curr_value = kStartValue;
  for (uint64_t i = 0; i < n; ++i) {
    curr_value = std::cos(curr_value);
  }
  benchmark::DoNotOptimize(curr_value);
}

static void BM_CpuTask(benchmark::State& state) {
  for (auto _ : state) {
    CpuTask();
  }
}
BENCHMARK(BM_CpuTask);

static void BM_LargeCapturedVariables(benchmark::State& state) {
  // Number of threads to use in thread pool.
  cb::ThreadPool pool(cb::ThreadPool::GetDefaultThreadPoolSize());

  // Make a large array of strings.
  std::vector<std::string> strings;
  const std::string kChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  for (int i = 0; i < 1000; ++i) {
    strings.emplace_back(500, kChars[i % kChars.size()]);
  }

  for (auto _ : state) {
    std::vector<std::future<void>> results;
    for (int i = 0; i < 100; ++i) {
      results.emplace_back(pool.ScheduleAndGetFuture([strings]() {
        const std::thread::id tid = std::this_thread::get_id();
        std::hash<std::thread::id> hasher;
        const int string_index = hasher(tid) % strings.size();
        const bool result =
            strings[string_index].find("C") != std::string::npos;
        benchmark::DoNotOptimize(result);
      }));
    }
    for (auto& f : results) {
      f.wait();
    }
  }
}
BENCHMARK(BM_LargeCapturedVariables)->UseRealTime();

static void BM_ThreadPoolUsage(benchmark::State& state) {
  const int num_threads = state.range(0);
  cb::ThreadPool pool(num_threads);
  for (auto _ : state) {
    std::array<std::future<void>, kNumTasks> futures;
    for (std::size_t i = 0; i < futures.size(); ++i) {
      futures[i] = pool.ScheduleAndGetFuture([]() { CpuTask(); });
    }
    for (auto& future : futures) {
      future.wait();
    }
  }
  state.SetItemsProcessed(state.iterations() * kNumTasks);
}
BENCHMARK(BM_ThreadPoolUsage)
    ->UseRealTime()
    ->RangeMultiplier(2)
    ->Range(1, 128)
    ->Arg(1000);

static void BM_AsyncUsage(benchmark::State& state) {
  for (auto _ : state) {
    std::array<std::future<void>, kNumTasks> futures;
    for (std::size_t i = 0; i < futures.size(); ++i) {
      futures[i] = std::async(std::launch::async, &CpuTask, kNumIterations);
    }
    for (auto& future : futures) {
      future.wait();
    }
  }
  state.SetItemsProcessed(state.iterations() * kNumTasks);
}
BENCHMARK(BM_AsyncUsage)->UseRealTime();

// Benchmark the overhead of waiting for a single a "no-op" function
// executed via std::async.
static void BM_AsyncOverhead(benchmark::State& state) {
  for (auto _ : state) {
    std::async(std::launch::async, []() {}).wait();
  }
}
BENCHMARK(BM_AsyncOverhead)->UseRealTime();

// Benchmark the overhead of waiting for a single a "no-op" function
// executed on a thread pool.
static void BM_ThreadpoolOverhead(benchmark::State& state) {
  // Number of threads to use in thread pool.
  constexpr int kNumThreads = 4;
  cb::ThreadPool pool(kNumThreads);
  for (auto _ : state) {
    pool.ScheduleAndGetFuture([]() {}).wait();
  }
}
BENCHMARK(BM_ThreadpoolOverhead)->UseRealTime();

int main(int argc, char** argv) {
  ::benchmark::Initialize(&argc, argv);
  ::benchmark::RunSpecifiedBenchmarks();
  return EXIT_SUCCESS;
}
