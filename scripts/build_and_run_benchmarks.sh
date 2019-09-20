#!/bin/bash

set -e

# Build unit tests.
bazel build -c opt test/...

# Run benchmarks
./bazel-bin/test/thread_pool_benchmarks  \
    --benchmark_min_time=10  # Seconds.
