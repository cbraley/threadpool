#ifndef SRC_MACROS_H_
#define SRC_MACROS_H_

// This file contains macros that we use to workaround some features that aren't
// available in C++11.

// We want to use std::invoke if C++17 is available, and fallback to "hand
// crafted" code if std::invoke isn't available.
#if __cplusplus >= 201703L
	#define INVOKE_MACRO(CALLABLE, ARGS_TYPE, ARGS)  std::invoke(CALLABLE, std::forward<ARGS_TYPE>(ARGS)...)
#elif __cplusplus >= 201103L
  // Update this with http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n4169.html.
	#define INVOKE_MACRO(CALLABLE, ARGS_TYPE, ARGS)  CALLABLE(std::forward<ARGS_TYPE>(ARGS)...)
#else
  #error ("C++ version is too old! C++98 is not supported.")
#endif

#endif // SRC_MACROS_H_
