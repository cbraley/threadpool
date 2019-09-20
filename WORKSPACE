workspace(name = "threadpool")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# gtest for unit tests.
git_repository(
    name = "googletest",
    remote = "https://github.com/google/googletest",
    commit = "bc2d0935b74917be0821bfd834472ed9cc4a3b5b",
)

# google benchmark library for microbenchmarks.
git_repository(
    name = "benchmark",
    remote = "https://github.com/google/benchmark",
    commit = "e7e3d976ef7d89ffc6bd6a53a6ea13ec35bb411d",
)
