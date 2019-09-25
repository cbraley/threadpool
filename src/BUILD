package(default_visibility = [
    "//visibility:public"
])

load("//config:variables.bzl", "COPTS")
load("//config:variables.bzl", "COPTS_CPP_17")

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
    name = "thread_pool_cpp_17",
    hdrs = [
        "thread_pool.h",
        "macros.h",
        ],
    srcs = ["thread_pool.cc"],
    deps = [],
    copts = COPTS_CPP_17,
)

