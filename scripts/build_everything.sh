#!/bin/bash

set -e

CPP_VERSION="c++17"

set -x

bazel build -c opt --cxxopt="-std=${CPP_VERSION}"  \
    test:thread_pool_test test:thread_pool_benchmarks

bazel build -c opt test:thread_pool_test_cpp11 test:thread_pool_test_cpp17 src/...
