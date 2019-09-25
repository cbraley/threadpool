# TODO(cbraley): Refactor!

    #  https://quuxplusone.github.io/blog/2018/12/06/dont-use-weverything/
COPTS = ["-W", "-Wall", "-Wextra", "-pedantic", "-pedantic-errors", "-Wshadow"]

COPTS_CPP_11 = COPTS + ["-std=c++11"]
COPTS_CPP_17 = COPTS + ["-std=c++11"]
