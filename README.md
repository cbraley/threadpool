# threadpool
Simple C++11 thread pool class with no external dependencies. 

## Build instructions

* Install (bazel)[https://bazel.build/]
* Run: 

```shell
bazel build -c opt test:thread_pool_test
./bazel-bin/test/threadpool_tests
```

* Write your own code that uses `threadpool.h`.
