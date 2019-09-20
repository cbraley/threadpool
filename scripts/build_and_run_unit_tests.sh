#!/bin/bash

set -e

# Build unit tests.
bazel build -c opt test/...

# Run unit tests.
./bazel-bin/test/thread_pool_test
