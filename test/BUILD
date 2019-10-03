load("//config:variables.bzl", "COPTS", "COPTS_CPP_11", "COPTS_CPP_17")

cc_binary(
    name = "thread_pool_test_cpp11",
    srcs = [
        "thread_pool_test.cc",
        "unit_test_main.cc",
    ],
    deps = [
        "//src:thread_pool_cpp11",
        "@googletest//:gtest",
    ],
    copts = COPTS_CPP_11,
)

cc_binary(
    name = "thread_pool_test_cpp17",
    srcs = [
        "thread_pool_test.cc",
        "unit_test_main.cc",
    ],
    deps = [
        "//src:thread_pool_cpp17",
        "@googletest//:gtest",
    ],
    copts = COPTS_CPP_17,
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
