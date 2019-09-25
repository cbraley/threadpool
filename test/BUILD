load("//config:variables.bzl", "COPTS", "COPTS_CPP_17", "COPTS_CPP_11")

cc_binary(
    name = "thread_pool_test",
    srcs = [
        "thread_pool_test.cc",
        "unit_test_main.cc",
    ],
    deps = [
        "//src:thread_pool",
        "@googletest//:gtest",
    ],
    copts = COPTS,
)

cc_binary(
    name = "thread_pool_benchmarks",
    srcs = [
        "thread_pool_benchmarks.cc",
    ],
    deps = [
        "//src:thread_pool",
        "@benchmark//:benchmark"
    ],
    copts = COPTS,
)
