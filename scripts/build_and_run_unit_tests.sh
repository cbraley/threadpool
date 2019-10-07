#!/bin/bash

set -e

# Parse C++ version flag.
CPP_VERSION="c++17"
if [ "$#" -gt 1 ]; then
  echo "Invalid arguments!!!!"
  echo "Expected usage:"
  echo "  ${0} [cpp_version]"
  echo "If you specify a value for [cpp_version], we will pass this string to "
  echo "the compiler as -std=[cpp_version]. If you don't specify a value, this "
  echo "will default to c++17. Consider using \"c++11\", \"c++17\", or "
  echo "\"c++14\"."
 	exit 1 
elif [ "$#" -eq 1 ]; then
  CPP_VERSION=${1}
  echo "Using C++ version \"${CPP_VERSION}\""
else
  echo "Using default C++ version \"${CPP_VERSION}\""
fi

set -x

# Build unit tests.
bazel build -c opt --cxxopt="-std=${CPP_VERSION}" test:thread_pool_test

# Run unit tests.
./bazel-bin/test/thread_pool_test
