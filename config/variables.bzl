COPTS = [
    "-std=c++17",
    #  https://quuxplusone.github.io/blog/2018/12/06/dont-use-weverything/
    "-W", "-Wall", "-Wextra", "-pedantic", "-pedantic-errors", "-Wshadow"
]
