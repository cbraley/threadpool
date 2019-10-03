#include <cstdio>
#include <cstdlib>

#include "gtest/gtest.h"

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  if (argc != 1) {
    std::cerr << "Unrecognized flags!" << std::endl;
    for (int i = 1; i < argc; ++i) {
      std::cerr << "  " << argv[i] << std::endl;
    }
    exit(EXIT_FAILURE);
  }
  return RUN_ALL_TESTS();
}
