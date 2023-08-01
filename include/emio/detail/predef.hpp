//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

namespace emio::detail {

// Helper macros for removing parentheses.
#define EMIO_Z_INTERNAL_DEPAREN(X) EMIO_Z_INTERNAL_ESC(EMIO_Z_INTERNAL_ISH_EMIO_INTERNAL X)
#define EMIO_Z_INTERNAL_ISH_EMIO_INTERNAL(...) EMIO_Z_INTERNAL_ISH_EMIO_INTERNAL __VA_ARGS__
#define EMIO_Z_INTERNAL_ESC(...) EMIO_Z_INTERNAL_ESC_(__VA_ARGS__)
#define EMIO_Z_INTERNAL_ESC_(...) EMIO_Z_INTERNAL_VAN##__VA_ARGS__
#define EMIO_Z_INTERNAL_VANEMIO_Z_INTERNAL_ISH_EMIO_INTERNAL

// Helper macros for generating an unique name.
#define EMIO_Z_INTERNAL_GLUE2(x, y) x##y
#define EMIO_Z_INTERNAL_GLUE(x, y) EMIO_Z_INTERNAL_GLUE2(x, y)
#define EMIO_Z_INTERNAL_UNIQUE_NAME EMIO_Z_INTERNAL_GLUE(_emio_try_unique_name_temporary, __COUNTER__)

#define EMIO_Z_INTERNAL_TRYV(name, expr)                     \
  do {                                                       \
    if (auto name = (expr); name.has_error()) [[unlikely]] { \
      return name.assume_error();                            \
    }                                                        \
  } while (0)

#define EMIO_Z_INTERNAL_TRY(name, var, expr) \
  auto name = (expr);                        \
  if (name.has_error()) [[unlikely]] {       \
    return name.assume_error();              \
  }                                          \
  EMIO_Z_INTERNAL_DEPAREN(var) = std::move(name).assume_value()

#if defined(__GNUC__) || defined(__GNUG__)
// Separate macro instead of std::is_constant_evaluated() because code will be optimized away even in debug if inlined.
#  define EMIO_Z_INTERNAL_IS_CONST_EVAL __builtin_is_constant_evaluated()
#  define EMIO_Z_INTERNAL_UNREACHABLE __builtin_unreachable()
#else
#  define EMIO_Z_INTERNAL_IS_CONST_EVAL std::is_constant_evaluated()
#  define EMIO_Z_INTERNAL_UNREACHABLE std::terminate()
#endif

#if defined(EMIO_ENABLE_DEV_ASSERT)
#  define EMIO_Z_DEV_ASSERT(...) \
    do {                         \
      if (!(__VA_ARGS__)) {      \
        std::terminate();        \
      }                          \
    } while (0)
#else
#  define EMIO_Z_DEV_ASSERT(...) static_cast<void>(__VA_ARGS__)
#endif

}  // namespace emio::detail
