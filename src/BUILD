package(default_visibility = [
    "//visibility:public"
])

load("//config:variables.bzl", "COPTS", "COPTS_CPP_11", "COPTS_CPP_17")

cc_library(
    name = "thread_pool",
    hdrs = [
        "thread_pool.h",
        "macros.h",
        ],
    srcs = ["thread_pool.cc"],
    deps = [],
    copts = COPTS,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "thread_pool_cpp11",
    hdrs = [
        "thread_pool.h",
        "macros.h",
        ],
    srcs = ["thread_pool.cc"],
    deps = [],
    copts = COPTS_CPP_11,
    visibility = ["//visibility:public"],
)

cc_library(
    name = "thread_pool_cpp17",
    hdrs = [
        "thread_pool.h",
        "macros.h",
        ],
    srcs = ["thread_pool.cc"],
    deps = [],
    copts = COPTS_CPP_17,
    visibility = ["//visibility:public"],
)
