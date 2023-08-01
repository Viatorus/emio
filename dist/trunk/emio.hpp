/*
  em{io} is a safe and fast high-level and low-level character input/output
  library for bare-metal and RTOS based embedded systems with a very small
  binary footprint.

  Copyright (c) 2021 - present, Toni Neubert (viatorus/emio)
  Copyright (c) 2012 - present, Victor Zverovich (fmtlib/fmt)

  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:

  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

  --- Optional exception to the license ---

  As an exception, if, as a result of your compiling your source code, portions
  of this Software are embedded into a machine-executable object form of such
  source code, you may redistribute such embedded portions in such object form
  without including the above copyright and permission notices.
 */

#ifndef EMIO_Z_MAIN_H
#define EMIO_Z_MAIN_H

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <cstdio>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <string_view>
#include <type_traits>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <string_view>
#include <type_traits>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <algorithm>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <iterator>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

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

namespace emio::detail {

constexpr bool isalpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

constexpr bool is_valid_number_base(int base) noexcept {
  return base >= 2 && base <= 36;
}

constexpr std::optional<int> char_to_digit(char c, int base) noexcept {
  if (c < '0') {
    return std::nullopt;
  }
  int res{};
  if (c >= 'a') {
    res = c - 'a' + 10;
  } else if (c >= 'A') {
    res = c - 'A' + 10;
  } else {
    res = c - '0';
  }
  if (res < base) {
    return res;
  }
  return std::nullopt;
}

constexpr char digit_to_char(int digit, int base, bool upper) noexcept {
  if (base >= 1 && digit >= 10) {
    if (upper) {
      return static_cast<char>(static_cast<int>('A') + (digit - 10));
    }
    return static_cast<char>(static_cast<int>('a') + (digit - 10));
  }
  return static_cast<char>(static_cast<int>('0') + digit);
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t count_digits_10(T number) noexcept {
  size_t count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++".
    if (number < 10) {
      return count;
    }
    if (number < 100) {
      return count + 1;
    }
    if (number < 1000) {
      return count + 2;
    }
    if (number < 10000) {
      return count + 3;
    }
    number /= 10000U;
    count += 4;
  }
}

template <size_t Base, typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t count_digits(T number) noexcept {
  if (number == 0) {
    return 1;
  }

  if constexpr (Base == 10) {
    return count_digits_10(number);
  } else if constexpr (Base == 2) {
    return std::bit_width(number);
  } else if constexpr (Base == 8) {
    return (std::bit_width(number) + 2) / 3;
  } else if constexpr (Base == 16) {
    return (std::bit_width(number) + 3) / 4;
  } else {
    size_t digit_cnt{1};
    for (number /= static_cast<T>(Base); number; number /= static_cast<T>(Base)) {
      ++digit_cnt;
    }
    return digit_cnt;
  }
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t get_number_of_digits(T number, int base) noexcept {
  if (number == 0) {
    return 1;
  }
  if (base == 10) {
    return count_digits<10>(number);
  }
  if (base == 16) {
    return count_digits<16>(number);
  }
  if (base == 2) {
    return count_digits<2>(number);
  }
  if (base == 8) {
    return count_digits<8>(number);
  }
  size_t digit_cnt{1};
  for (number /= static_cast<T>(base); number; number /= static_cast<T>(base)) {
    ++digit_cnt;
  }
  return digit_cnt;
}

template <typename T>
constexpr bool is_negative(T value) noexcept {
  if constexpr (std::is_signed_v<T>) {
    return value < 0;
  } else {
    return false;
  }
}

template <typename T>
constexpr int num_bits() noexcept {
  return std::numeric_limits<T>::digits;
}

template <typename T>
using int32_or_64 = std::conditional_t<num_bits<T>() <= 32, int32_t, int64_t>;

template <typename T>
using uint32_or_64 = std::conditional_t<num_bits<T>() <= 32, uint32_t, uint64_t>;

template <typename T>
  requires(std::is_integral_v<T>)
constexpr auto integer_upcast(T integer) {
  if constexpr (std::is_signed_v<T>) {
    return static_cast<int32_or_64<T>>(integer);
  } else {
    return static_cast<uint32_or_64<T>>(integer);
  }
}

template <typename T>
constexpr uint32_or_64<T> to_absolute(T number) noexcept {
  if constexpr (std::is_unsigned_v<T>) {
    return number;
  } else {
    if (is_negative(number)) {
      auto abs = static_cast<uint32_or_64<T>>(number);
      abs = T{} - abs;
      return abs;
    }
    return static_cast<uint32_or_64<T>>(number);
  }
}

template <typename T>
constexpr std::make_unsigned_t<T> to_unsigned(T number) noexcept {
  return static_cast<std::make_unsigned_t<T>>(number);
}

template <typename T>
constexpr std::make_signed_t<T> to_signed(T number) noexcept {
  return static_cast<std::make_signed_t<T>>(number);
}

// Converts value in the range [0, 100) to a string.
inline constexpr const char* digits2(size_t value) {
  // GCC generates slightly better code when value is pointer-size.
  return &"0001020304050607080910111213141516171819"
      "2021222324252627282930313233343536373839"
      "4041424344454647484950515253545556575859"
      "6061626364656667686970717273747576777879"
      "8081828384858687888990919293949596979899"[value * 2];
}

// Copies two characters from src to dst.
template <typename Char>
inline constexpr void copy2(Char* dst, const char* src) {
  if (!EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    memcpy(dst, src, 2);
  } else {
    *dst++ = static_cast<Char>(*src++);
    *dst = static_cast<Char>(*src);
  }
}

template <typename T, typename OutputIt>
  requires(std::is_unsigned_v<T>)
constexpr OutputIt write_decimal(T abs_number, OutputIt next) {
  if (abs_number == 0) {
    *(--next) = '0';
    return next;
  }
  // Write number from right to left.
  while (abs_number >= 100) {
    next -= 2;
    copy2(next, digits2(static_cast<size_t>(abs_number % 100)));
    abs_number /= 100;
  }
  if (abs_number < 10) {
    *--next = '0' + static_cast<char>(abs_number);
    return next;
  }
  next -= 2;
  copy2(next, digits2(static_cast<size_t>(abs_number)));
  return next;
}

template <typename T, typename OutputIt>
  requires(std::is_unsigned_v<T>)
constexpr OutputIt write_number(T abs_number, int base, bool upper, OutputIt next) {
  if (base == 10) {
    return write_decimal(abs_number, next);
  }
  if (abs_number == 0) {
    *(--next) = '0';
    return next;
  }
  // Write number from right to left.
  for (; abs_number; abs_number /= static_cast<T>(base)) {
    const char c = digit_to_char(static_cast<int>(abs_number % static_cast<T>(base)), base, upper);
    *(--next) = c;
  }
  return next;
}

inline constexpr size_t npos = std::string_view::npos;

constexpr std::string_view unchecked_substr(const std::string_view& str, size_t pos, size_t n = npos) noexcept {
  const size_t rlen = std::min(n, str.length() - pos);
  return {str.data() + pos, rlen};
}

template <typename Size>
constexpr char* fill_n(char* out, Size count, char value) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    for (Size i = 0; i < count; i++) {
      *out++ = value;
    }
    return out;
  } else {
    std::memset(out, value, to_unsigned(count));
    return out + count;
  }
}

template <typename Size>
constexpr char* copy_n(const char* in, Size count, char* out) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    for (Size i = 0; i < count; i++) {
      *out++ = *in++;
    }
    return out;
  } else {
    std::memcpy(out, in, to_unsigned(count));
    return out + count;
  }
}

}  // namespace emio::detail

namespace emio::detail {

/**
 * A constexpr vector with the bare minimum implementation and inlined storage.
 * @tparam Char The character type.
 * @tparam StorageSize The size of the inlined storage.
 */
template <typename Char, size_t StorageSize = 32>
class ct_vector {
 public:
  constexpr ct_vector() {
    if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
      fill_n(storage_.data(), storage_.size(), 0);
    }
  }

  ct_vector(const ct_vector&) = delete;
  ct_vector(ct_vector&&) = delete;
  ct_vector& operator=(const ct_vector&) = delete;
  ct_vector& operator=(ct_vector&&) = delete;

  constexpr ~ct_vector() noexcept {
    if (hold_external()) {
      delete[] data_;  // NOLINT(cppcoreguidelines-owning-memory)
    }
  }

  constexpr void reserve(size_t new_size) noexcept {
    if (new_size < StorageSize && !hold_external()) {
      size_ = new_size;
      return;
    }

    // Heavy pointer arithmetic because high level containers are not yet ready to use at constant evaluation.
    if (capacity_ < new_size) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      Char* new_data = new Char[new_size];  // NOLINT(cppcoreguidelines-owning-memory)
      copy_n(data_, size_, new_data);
      if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
        // Required at compile-time because another reserve could happen without previous write to the data.
        fill_n(new_data + size_, new_size - size_, 0);
      }
      std::swap(new_data, data_);
      capacity_ = new_size;
      if (new_data != storage_.data()) {
        delete[] new_data;  // NOLINT(cppcoreguidelines-owning-memory)
      }
    }
    size_ = new_size;
  }

  [[nodiscard]] constexpr size_t capacity() const noexcept {
    return capacity_;
  }

  [[nodiscard]] constexpr size_t size() const noexcept {
    return size_;
  }

  [[nodiscard]] constexpr Char* data() noexcept {
    return data_;
  }

  [[nodiscard]] constexpr const Char* data() const noexcept {
    return data_;
  }

 private:
  [[nodiscard]] constexpr bool hold_external() const noexcept {
    return data_ != storage_.data() && data_ != nullptr;
  }

  std::array<Char, StorageSize> storage_;
  Char* data_{storage_.data()};
  size_t size_{};
  size_t capacity_{StorageSize};
};

}  // namespace emio::detail

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <concepts>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <type_traits>

/**
 * Evaluates an expression. If successful, continues the execution. If unsuccessful, immediately returns from the
 * calling function.
 * @param expr The expression to evaluate.
 */
#define EMIO_TRYV(expr) EMIO_Z_INTERNAL_TRYV(EMIO_Z_INTERNAL_UNIQUE_NAME, expr)

/**
 * Evaluates an expression. If successful, assigns the value to a declaration. If unsuccessful, immediately returns from
 * the calling function.
 * @param var The declaration to assign the value to.
 * @param expr The expression to evaluate.
 */
#define EMIO_TRY(var, expr) EMIO_Z_INTERNAL_TRY(EMIO_Z_INTERNAL_UNIQUE_NAME, var, expr)

namespace emio {

/**
 * A list of possible I/O errors.
 */
enum class err {
  // success = 0,    ///< Internal used for no error.
  eof = 1,           ///< End of file (e.g. reaching the end of an output array).
  invalid_argument,  ///< A parameter is incorrect (e.g. the output base is invalid).
  invalid_data,      ///< The data is malformed (e.g. no digit where a digit was expected).
  out_of_range,      ///< The parsed value is not in the range representable by the type (e.g. parsing 578 as uint8_t).
  invalid_format,    ///< The format string is invalid.
};

/**
 * Returns the name of the possible I/O errors.
 * @param error The error.
 * @return The name.
 */
constexpr std::string_view to_string(err error) noexcept {
  using namespace std::string_view_literals;

  if (error == err{}) {
    return "no error"sv;
  }
  switch (error) {
  case err::eof:
    return "eof"sv;
  case err::invalid_argument:
    return "invalid argument"sv;
  case err::invalid_data:
    return "invalid data"sv;
  case err::out_of_range:
    return "out of range"sv;
  case err::invalid_format:
    return "invalid format"sv;
  }
  std::terminate();
}

/**
 * Used to indicates a successful operation without any value.
 */
inline constexpr struct {
} success;

/**
 * This exception type indicating an incorrect observation of value or error occurred by result<T>.
 *
 * No member functions are added in addition to std::logic_error. Typical .what() strings are:
 * - "no value" -> a value should be accessed but result<T> held an error
 * - "no error" -> a error should be accessed but result<T> held an value
 */
class bad_result_access : public std::logic_error {
 public:
  /**
   * Constructs the bad result access from a message.
   * @param msg The exception message.
   */
  explicit bad_result_access(const std::string_view& msg) : logic_error{msg.data()} {}
};

namespace detail {

#ifdef __EXCEPTIONS
inline constexpr bool exceptions_disabled = false;
#else
inline constexpr bool exceptions_disabled = true;
#endif

// Helper function to throw or terminate, depending on whether exceptions are globally enabled or not.
[[noreturn]] inline void throw_bad_result_access_or_terminate(const err error) noexcept(exceptions_disabled) {
#ifdef __EXCEPTIONS
  throw bad_result_access{to_string(error)};
#else
  static_cast<void>(error);
  std::terminate();
#endif
}

}  // namespace detail

/**
 * This class provides a way to store an optional value after an successful operation or an error after an failed
 * operation. The error type is emio::err.
 * @note See boost::outcome, std::expected or rust's std::result for the basic idea.
 * This provided API is a mix of each world.
 * @tparam T The value type to hold on success.
 */
template <typename Value>
class [[nodiscard]] result;

/**
 * Partial template specification of result<T> for any non-reference type.
 * @tparam Value The value type to hold on success.
 */
template <typename Value>
  requires(!std::is_reference_v<Value>)
class [[nodiscard]] result<Value> {
 public:
  /**
   * Explicit not default constructable.
   */
  constexpr result() = delete;

  /**
   * Constructs a result from a value to indicate an success.
   * @param value The value.
   */
  template <typename U>
    requires(std::is_constructible_v<Value, U>)
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload): Is guarded by require clause.
  constexpr explicit(!std::is_convertible_v<U, Value>) result(U&& value) : value_{std::forward<U>(value)} {}

  /**
   * Constructs a result from an error to indicate a failure.
   * @param error The error.
   */
  constexpr result(err error) : error_{error} {
    EMIO_Z_DEV_ASSERT(error != err{});
  }

  /**
   * Checks whether the object holds a value.
   * @return True if it holds a value, otherwise false.
   */
  constexpr explicit operator bool() const noexcept {
    return has_value();
  }

  /**
   * Checks whether the object holds a value.
   * @return True if it holds a value, otherwise false.
   */
  [[nodiscard]] constexpr bool has_value() const noexcept {
    return value_.has_value();
  }

  /**
   * Checks whether the object holds an error.
   * @return True if it holds an error, otherwise false.
   */
  [[nodiscard]] constexpr bool has_error() const noexcept {
    return !has_value();
  }

  /**
   * Returns a pointer to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  constexpr std::remove_reference_t<Value>* operator->() noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return &*value_;
  }

  /**
   * Returns a const pointer to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  constexpr const std::remove_reference_t<Value>* operator->() const noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return &*value_;
  }

  /**
   * Returns a reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value& operator*() & noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return *value_;
  }

  /**
   * Returns a const reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value& operator*() const& noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return *value_;
  }

  /**
   * Returns a rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value&& operator*() && noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return std::move(*value_);
  }

  /**
   * Returns a const rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value&& operator*() const&& noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return std::move(*value_);
  }

  /**
   * Returns a reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value& assume_value() & noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return *value_;  // NOLINT(bugprone-unchecked-optional-access): assumed
  }

  /**
   * Returns a const reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value& assume_value() const& noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return *value_;  // NOLINT(bugprone-unchecked-optional-access): assumed
  }

  /**
   * Returns a rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value&& assume_value() && noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return std::move(*value_);  // NOLINT(bugprone-unchecked-optional-access): assumed
  }

  /**
   * Returns a const rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value&& assume_value() const&& noexcept {
    EMIO_Z_DEV_ASSERT(has_value());
    return std::move(*value_);  // NOLINT(bugprone-unchecked-optional-access): assumed
  }

  /**
   * Returns a reference to the value.
   * @return The value.
   */
  constexpr Value& value() & noexcept(detail::exceptions_disabled) {
    if (value_.has_value()) [[likely]] {
      return *value_;
    }
    detail::throw_bad_result_access_or_terminate(error_);
  }

  /**
   * Returns a const reference to the value.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value& value() const& noexcept(detail::exceptions_disabled) {
    if (value_.has_value()) [[likely]] {
      return *value_;
    }
    detail::throw_bad_result_access_or_terminate(error_);
  }

  /**
   * Returns a rvalue reference to the value.
   * @return The value.
   */
  constexpr Value&& value() && noexcept(detail::exceptions_disabled) {
    if (value_.has_value()) [[likely]] {
      return std::move(*value_);
    }
    detail::throw_bad_result_access_or_terminate(error_);
  }

  /**
   * Returns a const rvalue reference to the value.
   * @return The value.
   */
  // NOLINTNEXTLINE(modernize-use-nodiscard): access check
  constexpr const Value&& value() const&& noexcept(detail::exceptions_disabled) {
    if (value_.has_value()) [[likely]] {
      return std::move(*value_);
    }
    detail::throw_bad_result_access_or_terminate(error_);
  }

  /**
   * Returns the error.
   * @note The behavior is undefined if this->has_error() is false.
   * @return The error.
   */
  [[nodiscard]] constexpr err assume_error() const noexcept {
    EMIO_Z_DEV_ASSERT(!has_value());
    return error_;
  }

  /**
   * Returns the error.
   * @return The error.
   * @throws bad_result_access exception or terminates (if exceptions are disabled) this->has_error() is false.
   */
  constexpr err error() const noexcept(detail::exceptions_disabled) {  // NOLINT(modernize-use-nodiscard): access check
    if (has_error()) [[likely]] {
      return error_;
    }
    detail::throw_bad_result_access_or_terminate(error_);
    return {};  // afl-c++ requires a return statement.
  }

  /**
   * Returns the contained value if *this has a value, otherwise returns default_value.
   * @param default_value The value to use in case *this is empty.
   * @return The current value if *this has a value, or default_value otherwise.
   */
  template <typename U>
  constexpr Value value_or(U&& default_value) const& {
    return value_.value_or(std::forward<U>(default_value));
  }

  /**
   * Returns the contained value if *this has a value, otherwise returns default_value.
   * @param default_value The value to use in case *this is empty.
   * @return The current value if *this has a value, or default_value otherwise.
   */
  template <typename U>
  constexpr Value value_or(U&& default_value) && {
    return value_.value_or(std::forward<U>(default_value));
  }

 private:
  std::optional<Value> value_{};
  err error_{};
};

/**
 * Partial template specification of result<T> for no type (void).
 * @note Although result<void> does not hold any value, the functions like has_value, assume_value and value are
 * provided with the exact same name to keep the API uniform.
 */
template <>
class [[nodiscard]] result<void> {
 public:
  /**
   * Explicit not default constructable.
   */
  constexpr result() = delete;

  /**
   * Constructs a result from the success value to indicate an success.
   * @param s The success value.
   */
  constexpr result([[maybe_unused]] decltype(success) s) {}

  /**
   * Constructs a result from an error to indicate a failure.
   * @param error The error.
   */
  constexpr result(err error) : error_{error} {
    EMIO_Z_DEV_ASSERT(error != err{});
  }

  /**
   * Constructs a result from another non void result and copies the success or error state.
   * @param other The other result.
   */
  template <typename T>
    requires(!std::is_void_v<T>)
  constexpr result(const result<T>& other) {
    if (other.has_error()) {
      error_ = other.assume_error();
    }
  }

  /**
   * Checks whether the object holds a value.
   * @return True if it holds a value, otherwise false.
   */
  constexpr explicit operator bool() const noexcept {
    return has_value();
  }

  /**
   * Checks whether the object holds a value.
   * @return True if it holds a value, otherwise false.
   */
  [[nodiscard]] constexpr bool has_value() const noexcept {
    return error_ == err{};
  }

  /**
   * Checks whether the object holds an error.
   * @return True if it holds an error, otherwise false.
   */
  [[nodiscard]] constexpr bool has_error() const noexcept {
    return !has_value();
  }

  /**
   * Does nothing for this type. Kept for consistent API.
   */
  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): Kept non-static for consistent API.
  constexpr void assume_value() const noexcept {
    // Nothing.
    EMIO_Z_DEV_ASSERT(has_value());
  }

  /**
   * If the result does have an error, this function throws/terminates.
   * @throws bad_result_access exception or terminates (if exceptions are disabled) if this->has_value() is false.
   */
  constexpr void value() const noexcept(detail::exceptions_disabled) {
    if (!has_value()) [[unlikely]] {
      detail::throw_bad_result_access_or_terminate(error_);
    }
  }

  /**
   * Returns the error.
   * @note The behavior is undefined if this->has_error() is false.
   * @return The error.
   */
  [[nodiscard]] constexpr err assume_error() const noexcept {
    EMIO_Z_DEV_ASSERT(!has_value());
    return error_;
  }

  /**
   * Returns the error.
   * @return The error.
   * @throws bad_result_access exception or terminates (if exceptions are disabled) this->has_error() is false.
   */
  constexpr err error() const noexcept(detail::exceptions_disabled) {  // NOLINT(modernize-use-nodiscard): access check
    if (has_error()) [[likely]] {
      return error_;
    }
    detail::throw_bad_result_access_or_terminate(error_);
    return {};  // afl-c++ requires a return statement.
  }

 private:
  err error_{};
};

namespace detail {

template <typename T>
struct is_result : std::false_type {};

template <typename T>
struct is_result<result<T>> : std::true_type {};

template <typename T>
inline constexpr bool is_result_v = is_result<T>::value;

}  // namespace detail

/**
 * Compares two result objects of maybe different but equality comparable types against each other.
 * @param left The left one.
 * @param right The right one.
 * @details Comparison against inequality will be automatically generated.
 * @return True if equal, otherwise false.
 * They are equal if:
 * - left.has_value() && right.has_value() && left.value() == right.value()
 * - left.has_error() && right.has_error() && left.error() == right.error()
 */
template <typename T, typename U>
  requires((std::is_void_v<T> && std::is_void_v<U>) || std::equality_comparable_with<T, U>)
constexpr bool operator==(const result<T>& left, const result<U>& right) {
  if (left.has_value() != right.has_value()) {
    return false;
  }
  if (left.has_value()) {
    if constexpr (std::is_void_v<T> && std::is_void_v<U>) {
      return true;
    } else {
      return left.assume_value() == right.assume_value();
    }
  }
  return left.assume_error() == right.assume_error();
}

/**
 * Compares a result object with an object of maybe different but equality comparable type against each other.
 * @param left The left one.
 * @param right The right one.
 * @details Comparison against inequality or right ==/!= left will be automatically generated.
 * @return True if equal, otherwise false.
 * They are equal if:
 * - left.has_value() && left.value() == right
 */
template <typename T, typename U>
  requires(!detail::is_result_v<U> && std::equality_comparable_with<T, U>)
constexpr bool operator==(const result<T>& left, const U& right) {
  if (left.has_value()) {
    return left.value() == right;
  }
  return false;
}

/**
 * Compares a result object against an error value for equality.
 * @param left The left one.
 * @param right The right one.
 * @details Comparison against inequality or right ==/!= left will be automatically generated.
 * @return True if equal, otherwise false.
 * They are equal if:
 * - left.has_error() && left.error() == right
 */
template <typename T>
constexpr bool operator==(const result<T>& left, const err right) {
  return left.has_error() && left.error() == right;
}

}  // namespace emio

namespace emio {

/**
 * This class provides the basic API and functionality for receiving a contiguous memory region of chars to write into.
 * @note Use a specific subclass for a concrete instantiation.
 */
class buffer {
 public:
  buffer(const buffer& other) = delete;
  buffer(buffer&& other) = delete;
  buffer& operator=(const buffer& other) = delete;
  buffer& operator=(buffer&& other) = delete;
  virtual constexpr ~buffer() = default;

  /**
   * Returns a write area with the requested size on success.
   * @param size The size the write area should have.
   * @return The write area with the requested size on success or eof if no write area is available.
   */
  constexpr result<std::span<char>> get_write_area_of(size_t size) noexcept {
    EMIO_TRY(const std::span<char> area, get_write_area_of_max(size));
    if (area.size() < size) {
      used_ -= area.size();
      return err::eof;
    }
    return area;
  }

  /**
   * Returns a write area which may be smaller than the requested size.
   * @note This function should be used to support subclasses with a limited internal buffer.
   * E.g. Writing a long string in chunks.
   * @param size The size the write area should maximal have.
   * @return The write area with the requested size as maximum on success or eof if no write area is available.
   */
  constexpr result<std::span<char>> get_write_area_of_max(size_t size) noexcept {
    // If there is enough remaining capacity in the current write area, return it.
    // Otherwise, request a new write area from the concrete implementation.
    // There is a special case for fixed size buffers. Since they cannot grow, they simply return the
    // remaining capacity or EOF, if hitting zero capacity.

    const size_t remaining_capacity = area_.size() - used_;
    if (remaining_capacity >= size || fixed_size_ == fixed_size::yes) {
      if (remaining_capacity == 0 && size != 0) {
        return err::eof;
      }
      const size_t max_size = std::min(remaining_capacity, size);
      const std::span<char> area = area_.subspan(used_, max_size);
      used_ += max_size;
      return area;
    }
    EMIO_TRY(const std::span<char> area, request_write_area(used_, size));
    used_ += area.size();
    return area;
  }

 protected:
  /// Flag to indicate if the buffer's size is fixed and cannot grow.
  enum class fixed_size : bool { no, yes };

  /**
   * Constructs the buffer.
   * @brief fixed Flag to indicate if the buffer's size is fixed and cannot grow.
   */
  constexpr explicit buffer(fixed_size fixed = fixed_size::no) noexcept : fixed_size_{fixed} {}

  /**
   * Requests a write area of the given size from a subclass.
   * @param used Already written characters into the current write area.
   * @param size The requested size of a new write area.
   * @return The write area with the requested size as maximum on success or eof if no write area is available.
   */
  virtual constexpr result<std::span<char>> request_write_area(const size_t used, const size_t size) noexcept {
    static_cast<void>(used);  // Keep params for documentation.
    static_cast<void>(size);
    return err::eof;
  }

  /**
   * Sets a new write area in the base class object to use.
   * @param area The new write area.
   */
  constexpr void set_write_area(const std::span<char> area) noexcept {
    area_ = area;
    used_ = 0;
  }

  /**
   * Returns the count of written characters of the current hold write area.
   * @return The count.
   */
  [[nodiscard]] constexpr size_t get_used_count() const noexcept {
    return used_;
  }

 private:
  fixed_size fixed_size_{fixed_size::no};
  size_t used_{};
  std::span<char> area_{};
};

/**
 * This class fulfills the buffer API by providing an endless growing buffer.
 * @tparam StorageSize The size of the inlined storage for small buffer optimization.
 */
template <size_t StorageSize = 32>
class memory_buffer final : public buffer {
 public:
  /**
   * Constructs and initializes the buffer with the internal storage size.
   */
  constexpr memory_buffer() : memory_buffer{0} {}

  /**
   * Constructs and initializes the buffer with the given capacity.
   * @param capacity The initial capacity.
   */
  constexpr explicit memory_buffer(const size_t capacity) {
    // Request at least the internal storage size.
    static_cast<void>(request_write_area(0, std::max(vec_.capacity(), capacity)));
  }

  constexpr memory_buffer(const memory_buffer&) = default;
  constexpr memory_buffer(memory_buffer&&) noexcept = default;
  constexpr memory_buffer& operator=(const memory_buffer&) = default;
  constexpr memory_buffer& operator=(memory_buffer&&) noexcept = default;
  constexpr ~memory_buffer() override = default;

  /**
   * Obtains a view over the underlying string object.
   * @return The view.
   */
  [[nodiscard]] constexpr std::string_view view() const noexcept {
    return {vec_.data(), used_ + this->get_used_count()};
  }

  /**
   * Obtains a copy of the underlying string object.
   * @return The string.
   */
  [[nodiscard]] std::string str() const {
    return std::string{view()};
  }

 protected:
  constexpr result<std::span<char>> request_write_area(const size_t used, const size_t size) noexcept override {
    const size_t new_size = vec_.size() + size;
    vec_.reserve(new_size);
    used_ += used;
    const std::span<char> area{vec_.data() + used_, size};
    this->set_write_area(area);
    return area;
  }

 private:
  size_t used_{};
  detail::ct_vector<char, StorageSize> vec_{};
};

/**
 * This class fulfills the buffer API by using a span over an contiguous range.
 */
class span_buffer : public buffer {
 public:
  /**
   * Constructs and initializes the buffer with an empty span.
   */
  constexpr span_buffer() : buffer{fixed_size::yes} {};

  /**
   * Constructs and initializes the buffer with the given span.
   * @param span The span.
   */
  constexpr explicit span_buffer(const std::span<char> span) : buffer{fixed_size::yes}, span_{span} {
    this->set_write_area(span_);
  }

  constexpr span_buffer(const span_buffer&) = delete;
  constexpr span_buffer(span_buffer&&) noexcept = delete;
  constexpr span_buffer& operator=(const span_buffer&) = delete;
  constexpr span_buffer& operator=(span_buffer&&) noexcept = delete;
  constexpr ~span_buffer() override = default;

  /**
   * Obtains a view over the underlying string object.
   * @return The view.
   */
  [[nodiscard]] constexpr std::string_view view() const noexcept {
    return {span_.data(), this->get_used_count()};
  }

  /**
   * Obtains a copy of the underlying string object.
   * @return The string.
   */
  [[nodiscard]] std::string str() const {
    return std::string{view()};
  }

 private:
  std::span<char> span_;
};

/**
 * This class fulfills the buffer API by providing a fixed-size storage.
 * @tparam StorageSize The size of the storage.
 */
template <size_t StorageSize>
class static_buffer : private std::array<char, StorageSize>, public span_buffer {
 public:
  /**
   * Constructs and initializes the buffer with the storage.
   */
  constexpr static_buffer() noexcept : span_buffer{std::span{*this}} {}

  constexpr static_buffer(const static_buffer&) = delete;
  constexpr static_buffer(static_buffer&&) noexcept = delete;
  constexpr static_buffer& operator=(const static_buffer&) = delete;
  constexpr static_buffer& operator=(static_buffer&&) noexcept = delete;
  constexpr ~static_buffer() override = default;

  // Note: We inherit from std::array to put the storage lifetime before span_buffer.
  // Clang will otherwise complain if the storage is a member variable and used during compile-time.
};

namespace detail {

inline constexpr size_t internal_buffer_size{256};

// Extracts a reference to the container from back_insert_iterator.
template <typename Container>
Container& get_container(std::back_insert_iterator<Container> it) {
  using bi_iterator = std::back_insert_iterator<Container>;
  struct accessor : bi_iterator {
    accessor(bi_iterator iter) : bi_iterator(iter) {}
    using bi_iterator::container;
  };
  return *accessor{it}.container;
}

// Helper struct to get the value type of different iterators.
template <typename T>
struct get_value_type {
  using type = typename std::iterator_traits<T>::value_type;
};

template <typename Container>
struct get_value_type<std::back_insert_iterator<Container>> {
  using type = typename Container::value_type;
};

template <typename Char, typename Traits>
struct get_value_type<std::ostreambuf_iterator<Char, Traits>> {
  using type = Char;
};

template <typename T>
using get_value_type_t = typename get_value_type<T>::type;

template <typename InputIt, typename OutputIt>
constexpr auto copy_str(InputIt it, InputIt end, OutputIt out) -> OutputIt {
  while (it != end) {
    *out++ = static_cast<char>(*it++);
  }
  return out;
}

}  // namespace detail

/**
 * This class template is used to create a buffer around different iterator types.
 */
template <typename Iterator>
class iterator_buffer;

/**
 * This class fulfills the buffer API by using an output iterator and an internal cache.
 * @tparam Iterator The output iterator type.
 */
template <typename Iterator>
  requires(std::input_or_output_iterator<Iterator> &&
           std::output_iterator<Iterator, detail::get_value_type_t<Iterator>>)
class iterator_buffer<Iterator> final : public buffer {
 public:
  /**
   * Constructs and initializes the buffer with the given output iterator.
   * @param it The output iterator.
   */
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized
  constexpr explicit iterator_buffer(Iterator it) : it_{it} {
    this->set_write_area(cache_);
  }

  iterator_buffer(const iterator_buffer&) = delete;
  iterator_buffer(iterator_buffer&&) = delete;
  iterator_buffer& operator=(const iterator_buffer&) = delete;
  iterator_buffer& operator=(iterator_buffer&&) = delete;

  /**
   * At destruction, the internal cache will be flushed to the output iterator.
   */
  constexpr ~iterator_buffer() override {
    flush();
  }

  /**
   * Flushes the internal cache to the output iterator.
   */
  constexpr void flush() noexcept {
    it_ = detail::copy_str(cache_.data(), cache_.data() + this->get_used_count(), it_);
    this->set_write_area(cache_);
  }

  /**
   * Flushes and returns the output iterator at the next write position.
   * @return The output iterator.
   */
  constexpr Iterator out() noexcept {
    flush();
    return it_;
  }

 protected:
  constexpr result<std::span<char>> request_write_area(const size_t /*used*/, const size_t size) noexcept override {
    flush();
    const std::span<char> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  Iterator it_;
  std::array<char, detail::internal_buffer_size> cache_;
};

/**
 * This class fulfills the buffer API by using an output pointer.
 * @tparam Iterator The output iterator type.
 */
template <typename OutputPtr>
  requires(std::input_or_output_iterator<OutputPtr*> &&
           std::output_iterator<OutputPtr*, detail::get_value_type_t<OutputPtr*>>)
class iterator_buffer<OutputPtr*> final : public buffer {
 public:
  /**
   * Constructs and initializes the buffer with the given output pointer.
   * @param ptr The output pointer.
   */
  constexpr explicit iterator_buffer(OutputPtr* ptr) : ptr_{ptr} {
    this->set_write_area({ptr, std::numeric_limits<size_t>::max()});
  }

  iterator_buffer(const iterator_buffer&) = delete;
  iterator_buffer(iterator_buffer&&) = delete;
  iterator_buffer& operator=(const iterator_buffer&) = delete;
  iterator_buffer& operator=(iterator_buffer&&) = delete;
  ~iterator_buffer() override = default;

  /**
   * Does nothing. Kept for uniformity with other iterator_buffer implementations.
   */
  constexpr void flush() noexcept {
    // Nothing.
  }

  /**
   * Returns the output pointer at the next write position.
   * @return The output pointer.
   */
  constexpr OutputPtr* out() {
    return ptr_ + this->get_used_count();
  }

 private:
  OutputPtr* ptr_;
};

/**
 * This class fulfills the buffer API by using the container of an contiguous back-insert iterator.
 * @tparam Container The container type of the back-insert iterator.
 */
template <typename Container>
  requires std::contiguous_iterator<typename Container::iterator>
class iterator_buffer<std::back_insert_iterator<Container>> final : public buffer {
 public:
  /**
   * Constructs and initializes the buffer with the given back-insert iterator.
   * @param it The output iterator.
   */
  constexpr explicit iterator_buffer(std::back_insert_iterator<Container> it) : container_{detail::get_container(it)} {
    static_cast<void>(request_write_area(0, std::min(container_.capacity(), detail::internal_buffer_size)));
  }

  iterator_buffer(const iterator_buffer&) = delete;
  iterator_buffer(iterator_buffer&&) = delete;
  iterator_buffer& operator=(const iterator_buffer&) = delete;
  iterator_buffer& operator=(iterator_buffer&&) = delete;

  /**
   * At destruction, the back-insert iterator will be flushed.
   */
  constexpr ~iterator_buffer() override {
    flush();
  }

  /**
   * Flushes the back-insert iterator by adjusting the size.
   */
  constexpr void flush() noexcept {
    container_.resize(used_ + this->get_used_count());
  }

  /**
   * Flushes and returns the back-insert iterator.
   * @return The back-insert iterator.
   */
  constexpr std::back_insert_iterator<Container> out() {
    flush();
    return std::back_inserter(container_);
  }

 protected:
  constexpr result<std::span<char>> request_write_area(const size_t used, const size_t size) noexcept override {
    const size_t new_size = container_.size() + size;
    container_.resize(new_size);
    used_ += used;
    const std::span<char> area{container_.data() + used_, new_size};
    this->set_write_area(area);
    return area.subspan(0, size);
  }

 private:
  size_t used_{};
  Container& container_;
};

template <typename Iterator>
iterator_buffer(Iterator&&) -> iterator_buffer<std::decay_t<Iterator>>;

/**
 * This class fulfills the buffer API by using a file stream and an internal cache.
 */
class file_buffer : public buffer {
 public:
  /**
   * Constructs and initializes the buffer with the given file stream.
   * @param file The file stream.
   */
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized
  constexpr explicit file_buffer(std::FILE* file) : file_{file} {
    this->set_write_area(cache_);
  }

  file_buffer(const file_buffer&) = delete;
  file_buffer(file_buffer&&) = delete;
  file_buffer& operator=(const file_buffer&) = delete;
  file_buffer& operator=(file_buffer&&) = delete;
  ~file_buffer() override = default;  // Doesn't flush because it could fail!

  /**
   * Flushes the internal cache to the file stream.
   * @note Does not flush the file stream itself!
   */
  result<void> flush() noexcept {
    const size_t written = std::fwrite(cache_.data(), sizeof(char), this->get_used_count(), file_);
    if (written != this->get_used_count()) {
      return err::eof;
    }
    this->set_write_area(cache_);
    return success;
  }

 protected:
  result<std::span<char>> request_write_area(const size_t /*used*/, const size_t size) noexcept override {
    EMIO_TRYV(flush());
    const std::span<char> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  std::FILE* file_;
  std::array<char, detail::internal_buffer_size> cache_;
};

namespace detail {

/**
 * A buffer that counts the number of characters written. Discards the output.
 */
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized.
class counting_buffer final : public buffer {
 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized.
  constexpr counting_buffer() noexcept = default;
  constexpr counting_buffer(const counting_buffer&) = delete;
  constexpr counting_buffer(counting_buffer&&) noexcept = delete;
  constexpr counting_buffer& operator=(const counting_buffer&) = delete;
  constexpr counting_buffer& operator=(counting_buffer&&) noexcept = delete;
  constexpr ~counting_buffer() override;

  /**
   * Calculates the number of Char's that were written.
   * @return The number of Char's.
   */
  [[nodiscard]] constexpr size_t count() const noexcept {
    return used_ + this->get_used_count();
  }

 protected:
  constexpr result<std::span<char>> request_write_area(const size_t used, const size_t size) noexcept override {
    used_ += used;
    const std::span<char> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  size_t used_{};
  std::array<char, detail::internal_buffer_size> cache_;
};

constexpr counting_buffer::~counting_buffer() = default;

}  // namespace detail

}  // namespace emio

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

namespace emio::detail {

inline constexpr bool needs_escape(uint32_t cp) {
  return cp < 0x20 || cp >= 0x7f || cp == '\'' || cp == '"' || cp == '\\';
}

inline constexpr size_t count_size_when_escaped(std::string_view sv) {
  size_t count = 0;
  for (const char c : sv) {
    if (!needs_escape(static_cast<uint32_t>(c))) {
      count += 1;
    } else if (c == '\n' || c == '\r' || c == '\t' || c == '\\' || c == '\'' || c == '"') {
      count += 2;
    } else {
      count += 2 + 2 * sizeof(char);  // \xAB...
    }
  }
  return count;
}

/*
 * Class which helps to escape a long string in smaller chunks.
 */
class write_escaped_helper {
 public:
  constexpr write_escaped_helper(std::string_view sv) noexcept : src_it_{sv.begin()}, src_end_{sv.end()} {}

  [[nodiscard]] constexpr size_t write_escaped(std::span<char> area) noexcept {
    char* dst_it = area.data();
    const char* const dst_end = area.data() + area.size();

    // Write remainder from temporary buffer.
    const auto write_remainder = [&, this] {
      while (remainder_it_ != remainder_end_ && dst_it != dst_end) {
        *(dst_it++) = *(remainder_it_++);
      }
    };
    write_remainder();

    while (src_it_ != src_end_) {
      if (dst_it == dst_end) {
        return static_cast<size_t>(dst_it - area.data());
      }
      const char c = *src_it_++;
      if (!needs_escape(static_cast<uint32_t>(c))) {
        *(dst_it++) = c;
      } else {
        *(dst_it++) = '\\';
        const auto remaining_space = static_cast<size_t>(dst_end - dst_it);
        if (remaining_space >= 3) {
          dst_it = write_escaped(c, dst_it);
        } else {
          // Write escaped sequence to remainder.
          remainder_it_ = remainder_storage_.begin();
          remainder_end_ = write_escaped(c, remainder_it_);
          // Write as much as possible into dst.
          write_remainder();
        }
      }
    }
    return static_cast<size_t>(dst_it - area.data());
  }

 private:
  [[nodiscard]] static inline constexpr char* write_escaped(const char c, char* out) noexcept {
    switch (c) {
    case '\n':
      *(out++) = 'n';
      return out;
    case '\r':
      *(out++) = 'r';
      return out;
    case '\t':
      *(out++) = 't';
      return out;
    case '\\':
      *(out++) = '\\';
      return out;
    case '\'':
      *(out++) = '\'';
      return out;
    case '"':
      *(out++) = '"';
      return out;
    default: {
      // Escape char zero filled like: \x05
      *(out++) = 'x';
      const auto abs = detail::to_absolute(detail::to_unsigned(c));
      const size_t number_of_digits = count_digits<16>(abs);
      // Fill up with zeros.
      for (size_t i = 0; i < 2 * sizeof(char) - number_of_digits; i++) {
        *(out++) = '0';
      }
      out += to_signed(number_of_digits);
      write_number(abs, 16, false, out);
      return out;
    }
    }
  }

  const char* src_it_;  // Input to encode.
  const char* src_end_;
  std::array<char, 4> remainder_storage_{};  // Remainder containing data for the next iteration.
  char* remainder_it_{};
  char* remainder_end_{};
};

}  // namespace emio::detail

namespace emio {

/**
 * This class operates on a buffer and allows writing sequences of characters or other kinds of data into it.
 */
class writer {
 public:
  /**
   * Constructs a writer with a given buffer.
   * @param buf The buffer.
   */
  constexpr writer(buffer& buf) noexcept : buf_{buf} {}

  /**
   * Returns the buffer.
   * @return The buffer.
   */
  [[nodiscard]] constexpr buffer& get_buffer() noexcept {
    return buf_;
  }

  /**
   * Writes a character into the buffer.
   * @param c The character.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char(const char c) noexcept {
    EMIO_TRY(const auto area, buf_.get_write_area_of(1));
    area[0] = c;
    return success;
  }

  /**
   * Writes a character n times into the buffer.
   * @param c The character.
   * @param n The number of times the character should be written.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char_n(const char c, const size_t n) noexcept {
    // Perform write in multiple chunks, to support buffers with an internal cache.
    size_t remaining_size = n;
    while (remaining_size != 0) {
      EMIO_TRY(const auto area, buf_.get_write_area_of_max(remaining_size));
      detail::fill_n(area.data(), area.size(), c);
      remaining_size -= area.size();
    }
    return success;
  }

  /**
   * Writes a character escaped into the buffer.
   * @param c The character.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char_escaped(const char c) noexcept {
    const std::string_view sv(&c, 1);
    return write_str_escaped('\'', sv);
  }

  /**
   * Writes a char sequence into the buffer.
   * @param sv The char sequence.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_str(const std::string_view sv) noexcept {
    // Perform write in multiple chunks, to support buffers with an internal cache.
    const char* ptr = sv.data();
    size_t remaining_size = sv.size();
    while (remaining_size != 0) {
      EMIO_TRY(const auto area, buf_.get_write_area_of_max(remaining_size));
      detail::copy_n(ptr, area.size(), area.data());
      remaining_size -= area.size();
    }
    return success;
  }

  /**
   * Writes a char sequence escaped into the buffer.
   * @param sv The char sequence.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_str_escaped(const std::string_view sv) noexcept {
    return write_str_escaped('"', sv);
  }

  /**
   * Format options for writing integers.
   */
  struct write_int_options {
    int base{10};            ///< The output base of the integer. Must be greater equal 2 and less equal 36.
    bool upper_case{false};  ///< If true, the letters are upper case, otherwise lower case.
  };

  /**
   * Writes an integer into the buffer.
   * @param integer The integer.
   * @param options The integer options.
   * @return invalid_argument if the requested output base is not supported or EOF if the buffer is to small.
   */
  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<void> write_int(const T integer,
                                   const write_int_options& options = default_write_int_options()) noexcept {
    // Reduce code generation by upcasting the integer.
    return write_int_impl(detail::integer_upcast(integer), options);
  }

 private:
  // Helper function since GCC and Clang complain about "member initializer for '...' needed within definition of
  // enclosing class". Which is a bug.
  static constexpr write_int_options default_write_int_options() noexcept {
    return {};
  }

  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<void> write_int_impl(const T integer, const write_int_options& options) noexcept {
    if (!detail::is_valid_number_base(options.base)) {
      return err::invalid_argument;
    }
    const auto abs_number = detail::to_absolute(integer);
    const bool negative = detail::is_negative(integer);
    const size_t number_of_digits =
        detail::get_number_of_digits(abs_number, options.base) + static_cast<size_t>(negative);

    EMIO_TRY(const auto area, buf_.get_write_area_of(number_of_digits));
    if (negative) {
      area[0] = '-';
    }
    detail::write_number(abs_number, options.base, options.upper_case,
                         area.data() + detail::to_signed(number_of_digits));
    return success;
  }

  constexpr result<void> write_str_escaped(const char quote, std::string_view sv) {
    // Perform escaping in multiple chunks, to support buffers with an internal cache.
    detail::write_escaped_helper helper{sv};
    size_t remaining_size = detail::count_size_when_escaped(sv);
    EMIO_TRY(auto area, buf_.get_write_area_of_max(remaining_size + 2 /*both quotes*/));
    // Start quote.
    area[0] = quote;
    area = area.subspan(1);

    while (true) {
      const size_t written = helper.write_escaped(area);
      remaining_size -= written;
      if (remaining_size == 0) {
        area = area.subspan(written);
        break;
      }
      EMIO_TRY(area, buf_.get_write_area_of_max(remaining_size + 1 /*end quote*/));
    }
    if (area.empty()) {
      EMIO_TRY(area, buf_.get_write_area_of_max(1 /*end quote*/));
    }
    // End quote.
    area[0] = quote;
    return success;
  }

  buffer& buf_;
};

}  // namespace emio

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <array>
#include <bit>
#include <exception>
#include <limits>

namespace emio::detail {

/**
 * A constexpr bitset with the bare minimum implementation.
 * @tparam Bits The number of bits.
 */
template <size_t Bits>
class bitset {
 private:
  using word_t = size_t;
  static constexpr size_t bits_per_word = sizeof(word_t) * 8;
  static constexpr size_t number_of_words = (Bits / bits_per_word) + (((Bits % bits_per_word) == 0) ? 0 : 1);

 public:
  /**
   * Checks if all bits are set to true.
   * @return true if all bits are set to true, otherwise false
   */
  [[nodiscard]] constexpr bool all() const noexcept {
    if constexpr (Bits <= 0) {
      return true;
    } else {
      for (size_t i = 0; i < number_of_words - 1; i++) {
        if (words_[i] != ~word_t{0}) {
          return false;
        }
      }
      constexpr word_t high_word_mask = get_high_word_mask();
      return words_[number_of_words - 1] == high_word_mask;
    }
  }

  /**
   * Checks if the first n bits are set to true.
   * @param n - number of bits
   * @return true if the first n bits are set to true, otherwise false
   */
  [[nodiscard, gnu::noinline]] constexpr bool all_first(size_t n) const noexcept {
    // Prevent inlining because of a GCC compiler-bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=106921
    if constexpr (Bits <= 0) {
      return n == 0;
    } else {
      if (n > Bits) {
        return false;
      }
      size_t i = 0;
      for (; n > bits_per_word; n -= bits_per_word, i++) {
        if (words_[i] != ~word_t{0}) {  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index): ensured by loop
          return false;
        }
      }
      word_t last_word = words_[i];  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index): ensured by loop
      for (; n != 0; n--) {
        if ((last_word & 1) != 1) {
          return false;
        }
        last_word >>= 1;
      }
      return true;
    }
  }

  /**
   * Returns the number of bits that the bitset holds.
   * @return the number of bits
   */
  [[nodiscard]] constexpr size_t size() const noexcept {
    return Bits;
  }

  /**
   * Sets a specific bit to true.
   * @param pos - the position of the bit
   */
  constexpr void set(size_t pos) noexcept {
    if (pos >= Bits) {
      std::terminate();
    }
    // Get index of pos in words and truncate pos to word bits per word.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index): ensured by check before
    words_[pos / bits_per_word] |= word_t{1} << (pos % bits_per_word);
  }

 private:
  static constexpr word_t get_high_word_mask() noexcept {
    word_t high_word_mask = (word_t{1} << (Bits % (bits_per_word))) - word_t{1};
    if (high_word_mask == 0) {
      return std::numeric_limits<word_t>::max();
    }
    return high_word_mask;
  }

  std::array<word_t, number_of_words> words_{};
};

}  // namespace emio::detail

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <array>
#include <span>
#include <string_view>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <algorithm>
#include <string>
#include <string_view>
#include <type_traits>

namespace emio {

/**
 * This class operates on a char sequence and allows reading and parsing from it.
 * The reader interprets the char sequence as finite input stream. After every successful operation the read position
 * moves on until the last char of the sequence has been consumed.
 */
class reader {
 public:
  /// The size type.
  using size_type = typename std::string_view::size_type;
  /// Special value to indicate the end of the view or an error by functions returning an index.
  static constexpr size_type npos = std::string_view::npos;

  /**
   * Constructs an empty reader.
   */
  constexpr reader() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr reader(std::string&&) = delete;
  constexpr reader(std::nullptr_t) = delete;
  constexpr reader(int) = delete;

  /**
   * Constructs the reader from any suitable char sequence.
   * @param input The char sequence.
   */
  template <typename Arg>
    requires(std::is_constructible_v<std::string_view, Arg>)
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload): Is guarded by require clause.
  constexpr explicit(!std::is_convertible_v<Arg, std::string_view>) reader(Arg&& input) noexcept
      : input_{std::forward<Arg>(input)} {}

  /**
   * Returns the current read position.
   * @return The read position.
   */
  [[nodiscard]] constexpr size_t pos() const noexcept {
    return pos_;
  }

  /**
   * Checks if the end of the stream has been reached.
   * @return True if reached and all chars are read, otherwise false.
   */
  [[nodiscard]] constexpr bool eof() const noexcept {
    return input_.size() - pos_ == 0;
  }

  /**
   * Returns the number of the not yet read chars of the stream.
   * @return The number of remaining chars.
   */
  [[nodiscard]] constexpr size_t cnt_remaining() const noexcept {
    return input_.size() - pos_;
  }

  /**
   * Obtains a view of the not yet read chars of the stream.
   * @return The view over the remaining chars.
   */
  [[nodiscard]] constexpr std::string_view view_remaining() const noexcept {
    const size_t x = input_.size() - pos_;
    if (x == 0) {
      return {};
    }
    return detail::unchecked_substr(input_, pos_);
  }

  /**
   * Reads all remaining chars from the stream.
   * @return The remaining chars. Could be empty.
   */
  [[nodiscard]] constexpr std::string_view read_remaining() noexcept {
    const std::string_view remaining_view = view_remaining();
    pop(remaining_view.size());
    return remaining_view;
  }

  /**
   * Pops one (default) or n chars from the reader.
   * @note Does never overflow.
   * @param cnt The number of chars to pop.
   */
  constexpr void pop(const size_t cnt = 1) noexcept {
    if (pos_ != input_.size()) {
      pos_ = std::min(pos_ + cnt, input_.size());
    }
  }

  /**
   * Makes the most recently extracted char available again.
   * @note Does never underflow.
   */
  constexpr void unpop(const size_t cnt = 1) noexcept {
    if (pos_ - cnt <= pos_) {
      pos_ = pos_ - cnt;
    } else {
      pos_ = 0;
    }
  }

  /**
   * Returns the next char from the stream without consuming it.
   * @return EOF if the end of the stream has been reached.
   */
  constexpr result<char> peek() const noexcept {
    const std::string_view remaining = view_remaining();
    if (!remaining.empty()) {
      return remaining[0];
    }
    return err::eof;
  }

  /**
   * Reads one char from the stream.
   * @return EOF if the end of the stream has been reached.
   */
  constexpr result<char> read_char() noexcept {
    const std::string_view remaining = view_remaining();
    if (!remaining.empty()) {
      pop();
      return remaining[0];
    }
    return err::eof;
  }

  /**
   * Reads n chars from the stream.
   * @param n The number of chars to read.
   * @return EOF if the end of the stream has been reached before reading n chars.
   */
  constexpr result<std::string_view> read_n_chars(const size_t n) noexcept {
    const std::string_view remaining = view_remaining();
    if (remaining.size() >= n) {
      pop(n);
      return std::string_view{remaining.data(), remaining.data() + n};
    }
    return err::eof;
  }

  /**
   * Parses an integer from the stream.
   * @tparam T The type of the integer.
   * @param base The input base of the integer. Must be greater equal 2 and less equal 36.
   * @return invalid_argument if the requested input base is not supported, EOF if the end of the stream has been
   * reached or invalid_data if the char sequence cannot be parsed as integer.
   */
  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<T> parse_int(const int base = 10) noexcept {
    // Store current read position.
    const size_t backup_pos = pos_;

    // Reduce code generation by upcasting the integer.
    using upcast_int_t = decltype(detail::integer_upcast(T{}));
    const result<upcast_int_t> res = parse_int_impl<upcast_int_t>(base);
    if (!res) {
      pos_ = backup_pos;
      return res.assume_error();
    }
    if constexpr (std::is_same_v<upcast_int_t, T>) {
      return res;
    } else {
      // Check if upcast int is within the integer type range.
      const upcast_int_t val = res.assume_value();
      if (val < std::numeric_limits<T>::min() || val > std::numeric_limits<T>::max()) {
        pos_ = backup_pos;
        return err::out_of_range;
      }
      return static_cast<T>(val);
    }
  }

  /**
   * Parse options for read_until operations.
   */
  struct read_until_options {
    bool include_delimiter{false};  ///< If true, the delimiter is part of the output char sequence, otherwise not.
    bool keep_delimiter{false};     ///< If true, the delimiter will be kept inside the stream, otherwise consumed.
    bool ignore_eof{false};  ///< If true, reaching EOF does result in a failed read, otherwise in a successfully read.
  };

  /**
   * Reads multiple chars from the stream until a given char as delimiter is reached or EOF (configurable).
   * @param delimiter The char delimiter.
   * @param options The read until options.
   * @return invalid_data if the delimiter hasn't been found and ignore_eof is set to true or EOF if the stream is
   * empty.
   */
  constexpr result<std::string_view> read_until_char(
      const char delimiter, const read_until_options& options = default_read_until_options()) noexcept {
    return read_until_pos(view_remaining().find(delimiter), options);
  }

  /**
   * Reads multiple chars from the stream until a given char sequence as delimiter is reached or EOF (configurable).
   * @param delimiter The char sequence.
   * @param options The read until options.
   * @return invalid_data if the delimiter hasn't been found and ignore_eof is set to true or EOF if the stream is
   * empty.
   */
  constexpr result<std::string_view> read_until_str(const std::string_view delimiter,
                                                    const read_until_options& options = default_read_until_options()) {
    return read_until_pos(view_remaining().find(delimiter), options, delimiter.size());
  }

  /**
   * Reads multiple chars from the stream until a char of a given group is reached or EOF (configurable).
   * @param group The char group.
   * @param options The read until options.
   * @return invalid_data if no char has been found and ignore_eof is set to True or EOF if the stream is empty.
   */
  constexpr result<std::string_view> read_until_any_of(
      const std::string_view group, const read_until_options& options = default_read_until_options()) noexcept {
    return read_until_pos(view_remaining().find_first_of(group), options);
  }

  /**
   * Reads multiple chars from the stream until no char of a given group is reached or EOF (configurable).
   * @param group The char group.
   * @param options The read until options.
   * @return invalid_data if a char not in the group has been found and ignore_eof is set to True or EOF if the stream
   * is empty.
   */
  constexpr result<std::string_view> read_until_none_of(
      const std::string_view group, const read_until_options& options = default_read_until_options()) noexcept {
    return read_until_pos(view_remaining().find_first_not_of(group), options);
  }

  /**
   * Reads multiple chars from the stream until a given predicate returns true or EOF is reached (configurable).
   * @param predicate The predicate taking a char and returning a bool.
   * @param options The read until options.
   * @return invalid_data if the predicate never returns true and ignore_eof is set to True or EOF if the stream is
   * empty.
   */
  template <typename Predicate>
    requires(std::is_invocable_r_v<bool, Predicate, char>)
  constexpr result<std::string_view> read_until(
      Predicate&& predicate,
      const read_until_options& options =
          default_read_until_options()) noexcept(std::is_nothrow_invocable_r_v<bool, Predicate, char>) {
    const std::string_view sv = view_remaining();
    const char* begin = sv.data();
    const char* end = sv.data() + sv.size();
    const char* it = std::find_if(begin, end, predicate);
    const intptr_t pos = (it != end) ? std::distance(begin, it) : 0;
    return read_until_pos(static_cast<size_t>(pos), options);
  }

  /**
   * Reads one char from the stream if the char matches the expected one.
   * @param c The expected char.
   * @return invalid_data if the chars don't match or EOF if the end of the stream has been reached.
   */
  constexpr result<char> read_if_match_char(const char c) noexcept {
    EMIO_TRY(const char p, peek());
    if (p == c) {
      pop();
      return c;
    }
    return err::invalid_data;
  }

  /**
   * Reads multiple chars from the stream if the chars matches the expected char sequence.
   * @param sv The expected char sequence.
   * @return invalid_data if the chars don't match or EOF if the end of the stream has been reached.
   */
  constexpr result<std::string_view> read_if_match_str(const std::string_view sv) noexcept {
    const std::string_view remaining = view_remaining();
    if (remaining.size() < sv.size()) {
      return err::eof;
    }
    if (remaining.starts_with(sv)) {
      pop(sv.size());
      return detail::unchecked_substr(remaining, 0, sv.size());
    }
    return err::invalid_data;
  }

 private:
  // Helper function since GCC and Clang complain about "member initializer for '...' needed within definition of
  // enclosing class". Which is a bug.
  [[nodiscard]] static constexpr read_until_options default_read_until_options() noexcept {
    return {};
  }

  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<T> parse_int_impl(const int base) {
    if (!detail::is_valid_number_base(base)) {
      return err::invalid_argument;
    }

    T value{};
    T maybe_overflowed_value{};
    T signed_flag{1};

    EMIO_TRY(char c, peek());  // NOLINT(misc-const-correctness): false-positive
    if (c == '-') {
      if constexpr (std::is_unsigned_v<T>) {
        return err::invalid_data;
      } else {
        signed_flag = -1;
        pop();
        EMIO_TRY(c, peek());
      }
    }
    std::optional<int> digit = detail::char_to_digit(c, base);
    if (!digit) {
      return err::invalid_data;
    }
    pop();

    while (true) {
      maybe_overflowed_value = value + static_cast<T>(*digit);
      if (maybe_overflowed_value < value) {
        return err::out_of_range;
      }
      value = maybe_overflowed_value;

      const result<char> res = peek();
      if (!res) {
        return signed_flag * value;
      }
      digit = detail::char_to_digit(res.value(), base);
      if (!digit) {
        return signed_flag * value;
      }
      pop();

      maybe_overflowed_value = value * static_cast<T>(base);
      if (maybe_overflowed_value < value) {
        return err::out_of_range;
      }
      value = maybe_overflowed_value;
    }
  }

  constexpr result<std::string_view> read_until_pos(size_t pos, const read_until_options& options,
                                                    const size_type delimiter_size = 1) noexcept {
    const std::string_view remaining = view_remaining();
    if (remaining.empty()) {
      return err::eof;
    }
    if (pos != npos) {
      if (!options.keep_delimiter) {
        pop(pos + delimiter_size);
      } else {
        pop(pos);
      }
      if (options.include_delimiter) {
        pos += delimiter_size;
      }
      return detail::unchecked_substr(remaining, 0, pos);
    }
    if (!options.ignore_eof) {
      pop(remaining.size());
      return remaining;
    }
    return err::invalid_data;
  }

  size_t pos_{};
  std::string_view input_;
};

}  // namespace emio

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <cstdint>

namespace emio::detail::format {

// "{" [arg_id] [":" (format_spec)]

// replacement_field ::=  "{" [arg_id] [":" (format_spec)] "}"
// arg_id            ::=  integer
// integer           ::=  digit+
// digit             ::=  "0"..."9"

// format_spec ::=  [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
// fill        ::=  <a character other than '{' or '}'>
// align       ::=  "<" | ">" | "^"
// sign        ::=  "+" | "-" | " "
// width       ::=  integer (<=int max)
// precision   ::=  integer (<=int max)
// type        ::=  "a" | "A" | "b" | "B" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G"| "o" | "O" | "p" | "s" | "x"
//                  | "X"

inline constexpr char no_sign = '\0';
inline constexpr int no_precision = -1;
inline constexpr char no_type = 0;

enum class alignment : uint8_t { none, left, center, right };

struct format_specs {
  char fill{' '};
  alignment align{alignment::none};
  char sign{no_sign};
  bool alternate_form{false};
  bool zero_flag{false};
  int32_t width{0};
  int32_t precision{no_precision};
  char type{no_type};
};

}  // namespace emio::detail::format

namespace emio::detail {

/**
 * Flag to enable/disable input validation if already done
 */
enum class input_validation { enabled, disabled };

template <typename T>
struct always_false : std::false_type {};

template <typename T>
inline constexpr bool always_false_v = always_false<T>::value;

namespace alternate_form {

inline constexpr std::string_view bin_lower{"0b"};
inline constexpr std::string_view bin_upper{"0B"};
inline constexpr std::string_view octal{"0"};
inline constexpr std::string_view octal_lower{"0o"};
inline constexpr std::string_view octal_upper{"0O"};
inline constexpr std::string_view hex_lower{"0x"};
inline constexpr std::string_view hex_upper{"0X"};

}  // namespace alternate_form

inline constexpr uint8_t no_more_args = std::numeric_limits<uint8_t>::max();

template <input_validation>
class parser_base {
 public:
  constexpr explicit parser_base(reader& format_rdr) noexcept : format_rdr_{format_rdr} {}

  parser_base(const parser_base&) = delete;
  parser_base(parser_base&&) = delete;
  parser_base& operator=(const parser_base&) = delete;
  parser_base& operator=(parser_base&&) = delete;

  virtual constexpr ~parser_base() = default;

  constexpr result<void> parse(uint8_t& arg_nbr) noexcept {
    while (true) {
      result<char> res = format_rdr_.read_char();
      if (res == err::eof) {
        return success;
      }
      if (!res) {
        return res;
      }
      char c = res.assume_value();
      if (c == '{') {
        EMIO_TRY(c, format_rdr_.peek());  // If failed: Incorrect escaped {.
        if (c != '{') {
          return parse_replacement_field(arg_nbr);
        }
        format_rdr_.pop();
      } else if (c == '}') {
        EMIO_TRY(c, format_rdr_.peek());
        if (c != '}') {
          // Not escaped }.
          return err::invalid_format;
        }
        format_rdr_.pop();
      }
      EMIO_TRYV(process(c));
    }
  }

 protected:
  virtual constexpr result<void> process(char c) noexcept = 0;

  reader& format_rdr_;

 private:
  constexpr result<void> parse_replacement_field(uint8_t& arg_nbr) noexcept {
    EMIO_TRYV(parse_field_name(arg_nbr));

    EMIO_TRY(const char c, format_rdr_.peek());
    if (c == '}') {
      return success;
    }
    if (c == ':') {  // Format specs.
      // Format specs are parsed with known argument type later.
      format_rdr_.pop();
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> parse_field_name(uint8_t& arg_nbr) noexcept {
    EMIO_TRY(const char c, format_rdr_.peek());
    if (detail::isdigit(c)) {               // Positional argument.
      if (use_positional_args_ == false) {  // If first argument was positional -> failure.
        return err::invalid_format;
      }
      EMIO_TRY(arg_nbr, format_rdr_.template parse_int<uint8_t>());
      use_positional_args_ = true;
    } else {
      if (use_positional_args_ == true) {
        return err::invalid_format;
      }
      use_positional_args_ = false;
      // None positional argument. Increase arg_nbr after each format specifier.
      arg_nbr = increment_arg_number_++;
    }
    return success;
  }

  std::optional<bool> use_positional_args_{};
  uint8_t increment_arg_number_{};
};

template <>
class parser_base<input_validation::disabled> {
 public:
  constexpr explicit parser_base(reader& format_rdr) noexcept : format_rdr_{format_rdr} {}

  parser_base(const parser_base& other) = delete;
  parser_base(parser_base&& other) = delete;
  parser_base& operator=(const parser_base& other) = delete;
  parser_base& operator=(parser_base&& other) = delete;

  virtual constexpr ~parser_base() = default;

  constexpr result<void> parse(uint8_t& arg_nbr) noexcept {
    while (true) {
      result<char> res = format_rdr_.read_char();
      if (res == err::eof) {
        return success;
      }
      char c = res.assume_value();
      if (c == '{') {
        c = format_rdr_.peek().assume_value();
        if (c != '{') {
          return parse_replacement_field(arg_nbr);
        }
        format_rdr_.pop();
      } else if (c == '}') {
        format_rdr_.pop();
      }
      EMIO_TRYV(process(c));
    }
  }

 protected:
  virtual constexpr result<void> process(char c) noexcept = 0;

  reader& format_rdr_;

 private:
  constexpr result<void> parse_replacement_field(uint8_t& arg_nbr) noexcept {
    parse_field_name(arg_nbr);
    const char c = format_rdr_.peek().assume_value();
    if (c == '}') {
      return success;
    }
    format_rdr_.pop();
    return success;
  }

  constexpr void parse_field_name(uint8_t& arg_nbr) noexcept {
    const char c = format_rdr_.peek().assume_value();
    if (detail::isdigit(c)) {  // Positional argument.
      arg_nbr = format_rdr_.template parse_int<uint8_t>().assume_value();
      use_positional_args_ = true;
    } else {
      use_positional_args_ = false;
      // None positional argument. Increase arg_nbr after each format specifier.
      arg_nbr = increment_arg_number_++;
    }
  }

  std::optional<bool> use_positional_args_{};
  uint8_t increment_arg_number_{};
};

}  // namespace emio::detail

//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

// This implementation is based on:
// https://github.com/rust-lang/rust/blob/71ef9ecbdedb67c32f074884f503f8e582855c2f/library/core/src/num/flt2dec/strategy/dragon.rs

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <optional>
#include <span>
#include <type_traits>

//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

// This implementation is based on:
// https://github.com/rust-lang/rust/blob/71ef9ecbdedb67c32f074884f503f8e582855c2f/library/core/src/num/bignum.rs

#include <algorithm>
#include <array>
#include <cstdint>
#include <exception>
#include <span>

namespace emio::detail {

struct carrying_add_result_t {
  uint32_t value;
  bool carry;

  friend constexpr bool operator==(const carrying_add_result_t& lhs,
                                   const carrying_add_result_t& rhs) noexcept = default;
};

inline constexpr carrying_add_result_t carrying_add(uint32_t a, uint32_t b, bool carry) {
  const uint32_t v1 = a + b;
  const bool carry1 = v1 < a;
  const uint32_t v2 = v1 + static_cast<uint32_t>(carry);
  const bool carry2 = v2 < v1;
  return {v2, carry1 || carry2};
}

struct borrowing_sub_result_t {
  uint32_t value;
  bool borrow;

  friend constexpr bool operator==(const borrowing_sub_result_t& lhs,
                                   const borrowing_sub_result_t& rhs) noexcept = default;
};

inline constexpr borrowing_sub_result_t borrowing_sub(uint32_t a, uint32_t b, bool borrow) {
  const uint32_t v1 = a - b;
  const bool borrow1 = v1 > a;
  const uint32_t v2 = v1 - static_cast<uint32_t>(borrow);
  const bool borrow2 = v2 > v1;
  return {v2, borrow1 || borrow2};
}

struct carrying_mul_result_t {
  uint32_t value;
  uint32_t carry;

  friend constexpr bool operator==(const carrying_mul_result_t& lhs,
                                   const carrying_mul_result_t& rhs) noexcept = default;
};

inline constexpr carrying_mul_result_t carrying_mul(uint32_t a, uint32_t b, uint32_t carry) {
  const uint64_t v1 = static_cast<uint64_t>(a) * b + carry;
  const auto v2 = static_cast<uint32_t>(v1);
  const auto carry1 = static_cast<uint32_t>(v1 >> 32U);
  return {v2, carry1};
}

/// Stack-allocated arbitrary-precision (up to certain limit) integer.
class bignum {
 public:
  static constexpr size_t max_blocks = 34;

  static constexpr bignum from(size_t sz, const std::array<uint32_t, max_blocks>& b) {
    bignum bn{};
    bn.size_ = sz;
    bn.base_ = b;
    return bn;
  }

  constexpr explicit bignum() noexcept = default;

  /// Makes a bignum from one digit.
  template <typename T>
    requires(std::is_unsigned_v<T> && sizeof(T) <= sizeof(uint32_t))
  constexpr explicit bignum(T v) noexcept : base_{{v}} {}

  /// Makes a bignum from `u64` value.
  template <typename T>
    requires(std::is_unsigned_v<T> && sizeof(T) == sizeof(uint64_t))
  constexpr explicit bignum(T v) noexcept : base_{{static_cast<uint32_t>(v), static_cast<uint32_t>(v >> 32)}} {
    size_ += static_cast<size_t>(base_[1] > 0);
  }

  /// Returns the internal digits as a slice `[a, b, c, ...]` such that the numeric
  /// value is `a + b * 2^W + c * 2^(2W) + ...` where `W` is the number of bits in
  /// the digit type.
  constexpr std::span<uint32_t> digits() noexcept {
    return {base_.data(), size_};
  }

  /// Returns the `i`-th bit where bit 0 is the least significant one.
  /// In other words, the bit with weight `2^i`.
  [[nodiscard]] constexpr uint8_t get_bit(size_t i) const noexcept {
    const size_t digitbits = 32;
    const auto d = i / digitbits;
    const auto b = i % digitbits;
    return static_cast<uint8_t>((base_[d] >> b) & 1U);
  }

  /// Returns `true` if the bignum is zero.
  [[nodiscard]] constexpr bool is_zero() const noexcept {
    return std::all_of(base_.begin(), base_.end(), [](uint32_t v) {
      return v == 0;
    });
  }

  // add
  // add_small
  constexpr bignum& add_small(uint32_t other) noexcept {
    return add_small_at(0, other);
  }

  constexpr bignum& add_small_at(size_t index, uint32_t other) noexcept {
    size_t i = index;
    auto res = carrying_add(base_[i], other, false);
    base_[i] = res.value;
    i += 1;
    for (; res.carry && (i < base_.size()); i++) {
      res = carrying_add(base_[i], 0, res.carry);
      base_[i] = res.value;
    }
    EMIO_Z_DEV_ASSERT(!res.carry);
    size_ = i;
    return *this;
  }

  constexpr bignum& add(const bignum& other) noexcept {
    carrying_add_result_t res{0, false};
    size_t i = 0;
    for (; (i < other.size_) || (res.carry && (i < base_.size())); i++) {
      res = carrying_add(base_[i], other.base_[i], res.carry);
      base_[i] = res.value;
    }
    EMIO_Z_DEV_ASSERT(!res.carry);
    if (i > size_) {
      size_ = i;
    }
    return *this;
  }

  /// Subtracts `other` from itself and returns its own mutable reference.
  constexpr bignum& sub_small(uint32_t other) noexcept {
    auto res = borrowing_sub(base_[0], other, false);
    base_[0] = res.value;
    size_t i = 1;
    for (; res.borrow && (i < base_.size()); i++) {
      res = borrowing_sub(base_[i], 0, res.borrow);
      base_[i] = res.value;
    }
    EMIO_Z_DEV_ASSERT(!res.borrow);
    if (i == size_ && size_ != 1) {
      size_ -= 1;
    }
    return *this;
  }

  /// Subtracts `other` from itself and returns its own mutable reference.
  constexpr bignum& sub(const bignum& other) noexcept {
    EMIO_Z_DEV_ASSERT(size_ >= other.size_);
    if (size_ == 0) {
      return *this;
    }
    borrowing_sub_result_t res{0, false};
    for (size_t i = 0; i < size_; i++) {
      res = borrowing_sub(base_[i], other.base_[i], res.borrow);
      base_[i] = res.value;
    }
    EMIO_Z_DEV_ASSERT(!res.borrow);
    do {
      if (base_[size_ - 1] != 0) {
        break;
      }
    } while (--size_ != 0);
    return *this;
  }

  /// Multiplies itself by a digit-sized `other` and returns its own
  /// mutable reference.
  constexpr bignum& mul_small(uint32_t other) noexcept {
    return muladd_small(other, 0);
  }

  constexpr bignum& muladd_small(uint32_t other, uint32_t carry) noexcept {
    carrying_mul_result_t res{0, carry};
    for (size_t i = 0; i < size_; i++) {
      res = carrying_mul(base_[i], other, res.carry);
      base_[i] = res.value;
    }
    if (res.carry > 0) {
      base_[size_] = res.carry;
      size_ += 1;
    }
    return *this;
  }

  [[nodiscard]] bignum mul(const bignum& other) const noexcept {
    const auto& bn_max = size_ > other.size_ ? *this : other;
    const auto& bn_min = size_ > other.size_ ? other : *this;

    bignum prod{};
    for (size_t i = 0; i < bn_min.size_; i++) {
      carrying_mul_result_t res{0, 0};
      for (size_t j = 0; j < bn_max.size_; j++) {
        res = carrying_mul(bn_min.base_[i], bn_max.base_[j], res.carry);
        prod.add_small_at(i + j, res.value);
      }
      if (res.carry > 0) {
        prod.add_small_at(i + bn_max.size_, res.carry);
      }
    }
    return prod;
  }

  constexpr bignum& mul_digits(std::span<const uint32_t> other) noexcept {
    const auto& bn_max = size_ > other.size() ? digits() : other;
    const auto& bn_min = size_ > other.size() ? other : digits();

    bignum prod{};
    for (size_t i = 0; i < bn_min.size(); i++) {
      carrying_mul_result_t res{0, 0};
      for (size_t j = 0; j < bn_max.size(); j++) {
        res = carrying_mul(bn_min[i], bn_max[j], res.carry);
        prod.add_small_at(i + j, res.value);
      }
      if (res.carry > 0) {
        prod.add_small_at(i + bn_max.size(), res.carry);
      }
    }
    *this = prod;
    return *this;
  }

  /// Multiplies itself by `5^e` and returns its own mutable reference.
  constexpr bignum& mul_pow5(size_t k) noexcept {
    // Multiply with the largest single-digit power as long as possible.
    while (k >= 13) {
      mul_small(1220703125);
      k -= 13;
    }
    // Stop if nothing left.
    if (k == 0) {
      return *this;
    }
    // Finish off the remainder.
    uint32_t rest_power{5};
    while (--k > 0) {
      rest_power *= 5;
    }
    return mul_small(rest_power);
  }

  /// Divides itself by a digit-sized `other` and returns its own
  /// mutable reference *and* the remainder.
  constexpr uint32_t div_rem_small(uint32_t other) {
    uint64_t borrow = 0;
    for (size_t i = size_; i > 0; i--) {
      const uint64_t v = (base_[i - 1] + (borrow << 32U));
      const uint64_t res = v / other;
      base_[i - 1] = static_cast<uint32_t>(res);
      borrow = v - res * other;
    }
    return static_cast<uint32_t>(borrow);
  }

  /// Multiplies itself by `2^exp` and returns its own mutable reference.
  constexpr bignum& mul_pow2(size_t exp) noexcept {
    const size_t digits = exp / 32;
    const size_t bits = exp % 32;

    if (digits > 0) {
      for (size_t i = size_; i > 0; --i) {
        base_[i + digits - 1] = base_[i - 1];
      }
      for (size_t i = 0; i < digits; i++) {
        base_[i] = 0;
      }
      size_ += digits;
    }
    if (bits > 0) {
      uint32_t overflow = 0;
      size_t i = 0;
      for (; i < size_; i++) {
        auto res = static_cast<uint64_t>(base_[i]) << bits;
        base_[i] = static_cast<uint32_t>(res) + overflow;
        overflow = static_cast<uint32_t>(res >> 32);
      }
      if (overflow > 0) {
        base_[i] = overflow;
        size_ += 1;
      }
    }
    return *this;
  }

  [[nodiscard]] constexpr std::strong_ordering operator<=>(const bignum& other) const noexcept {
    if (size_ > other.size_) {
      return std::strong_ordering::greater;
    }
    if (size_ < other.size_) {
      return std::strong_ordering::less;
    }
    for (size_t i = size_; i > 0; i--) {
      if (base_[i - 1] > other.base_[i - 1]) {
        return std::strong_ordering::greater;
      }
      if (base_[i - 1] < other.base_[i - 1]) {
        return std::strong_ordering::less;
      }
    }
    return std::strong_ordering::equal;
  }

  constexpr bool operator==(const bignum& other) const noexcept = default;

 private:
  /// Number of "digits" used in base_.
  size_t size_{1};
  /// Digits. `[a, b, c, ...]` represents `a + b*2^W + c*2^(2W) + ...`
  /// where `W` is the number of bits in the digit type.
  std::array<uint32_t, max_blocks> base_{};
};

}  // namespace emio::detail

//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

// This implementation is based on:
// https://github.com/rust-lang/rust/blob/71ef9ecbdedb67c32f074884f503f8e582855c2f/library/core/src/num/flt2dec/decoder.rs

#include <bit>
#include <cstdint>
#include <cstring>
#include <limits>
#include <type_traits>

namespace emio::detail::format {

struct finite_result_t {
  uint64_t mant{};
  uint64_t minus{};
  uint64_t plus{};
  int16_t exp{};
  bool inclusive{};
};

enum class category { zero, finite, infinity, nan };

struct decode_result_t {
  bool negative{};
  format::category category{};
  finite_result_t finite{};  // Only valid if category is finite.
};

inline constexpr decode_result_t decode(double value) noexcept {
  decode_result_t res{};

  using bits_type = uint64_t;
  const auto bits = std::bit_cast<bits_type>(value);

  res.negative = bits >> 63 != 0;
  if (value == 0) {
    return res;
  }

  // Exponent bias + mantissa shift
  res.finite.exp = static_cast<int16_t>(((bits >> 52) & 0x7ff) - (1023 + 52));
  res.finite.mant = res.finite.exp == -1075 ? (bits & 0xfffffffffffff) << 1 : (bits & 0xfffffffffffff);
  res.finite.inclusive = (res.finite.mant & 1) == 0;

  if (res.finite.exp == 972) {  // non-numbers.
    if (res.finite.mant == 0) {
      res.category = category::infinity;
    } else {
      res.category = category::nan;
    }
  } else {
    res.category = category::finite;
    res.finite.minus = 1;
    res.finite.plus = 1;
    if (res.finite.exp != -1075) {  // Norm.
      res.finite.mant |= 0x10000000000000;
      constexpr auto minnorm = std::bit_cast<bits_type>(std::numeric_limits<double>::min());
      if (res.finite.mant == minnorm) {
        res.finite.plus = 2;
        res.finite.mant <<= 2;
        res.finite.exp -= 2;
      } else {
        res.finite.mant <<= 1;
        res.finite.exp -= 1;
      }
    }
  }

  return res;
}

}  // namespace emio::detail::format

namespace emio::detail::format {

inline constexpr int16_t estimate_scaling_factor(uint64_t mant, int16_t exp) noexcept {
  // 2^(nbits-1) < mant <= 2^nbits if mant > 0
  const int nbits = 64 - std::countl_zero(mant - 1);
  // 1292913986 = floor(2^32 * log_10 2)
  // therefore this always underestimates (or is exact), but not much.
  return static_cast<int16_t>((static_cast<int64_t>(nbits + exp) * 1292913986) >> 32);
}

inline constexpr std::optional<char> round_up(std::span<char> d) noexcept {
  const auto end = d.rend();
  auto it = std::find_if(d.rbegin(), end, [](char c) {
    return c != '9';
  });
  if (it != end) {
    // d[i+1..n] is all nines.
    auto i = static_cast<size_t>(std::distance(it, end) - 1);
    d[i] += 1;  // Round up.
    for (size_t j = i + 1; j < d.size(); j++) {
      d[j] = '0';
    }
    return std::nullopt;
  } else if (!d.empty()) {
    // 999..999 rounds to 1000..000 with an increased exponent
    d[0] = '1';
    for (char& c : d.subspan(1)) {
      c = '0';
    }
    return '0';
  }
  // an empty buffer rounds up (a bit strange but reasonable)
  return '1';
}

struct format_fp_result_t {
  std::span<const char> digits;
  int16_t exp;
};

enum class format_exact_mode { significand_digits, decimal_point };

inline constexpr format_fp_result_t format_exact(const finite_result_t& dec, emio::buffer& buf, format_exact_mode mode,
                                                 int16_t number_of_digits) noexcept {
  EMIO_Z_DEV_ASSERT(dec.mant > 0);
  EMIO_Z_DEV_ASSERT(dec.minus > 0);
  EMIO_Z_DEV_ASSERT(dec.plus > 0);

  // estimate `k_0` from original inputs satisfying `10^(k_0-1) < v <= 10^(k_0+1)`.
  int16_t k = estimate_scaling_factor(dec.mant, dec.exp);

  // `v = mant / scale`.
  auto mant = bignum(dec.mant);
  auto scale = bignum(1U);

  size_t s2 = 0;
  size_t s5 = 0;
  size_t m2 = 0;
  size_t m5 = 0;

  if (dec.exp < 0) {
    s2 = static_cast<size_t>(-dec.exp);
  } else {
    m2 += static_cast<size_t>(dec.exp);
  }

  // divide `mant` by `10^k`. now `scale / 10 < mant <= scale * 10`.
  if (k >= 0) {
    s2 += static_cast<size_t>(k);
    s5 += static_cast<size_t>(k);
  } else {
    m2 += static_cast<size_t>(-k);
    m5 += static_cast<size_t>(-k);
  }

  scale.mul_pow5(s5);
  scale.mul_pow2(s2);

  mant.mul_pow5(m5);
  mant.mul_pow2(m2);

  // calculate required buffer size
  size_t len{};
  size_t extra_len{};
  if (mode == format_exact_mode::significand_digits) {
    len = static_cast<size_t>(number_of_digits);
  } else if ((k + number_of_digits) >= 0) {
    len = static_cast<size_t>(k + number_of_digits);
    extra_len = 1;
  }

  // fixup estimation
  // in order to keep the fixed-size bignum, we actually use `mant + floor(plus) >= scale`.
  // we are not actually modifying `scale`, since we can skip the initial multiplication instead.
  // again with the shortest algorithm, `d[0]` can be zero but will be eventually rounded up.
  // if we are working with the last-digit limitation, we need to shorten the buffer
  // before the actual rendering in order to avoid double rounding.
  // note that we have to enlarge the buffer again when rounding up happens!
  if (mant >= scale) {
    k += 1;
    len += extra_len;
  } else {
    mant.mul_small(10);
  }

  auto dst = buf.get_write_area_of(len).value();

  if (len > 0) {
    // cache `(2, 4, 8) * scale` for digit generation.
    bignum scale2 = scale;
    scale2.mul_pow2(1);
    bignum scale4 = scale;
    scale4.mul_pow2(2);
    bignum scale8 = scale;
    scale8.mul_pow2(3);

    for (size_t i = 0; i < len; i++) {
      if (mant.is_zero()) {
        // following digits are all zeroes, we stop here
        // do *not* try to perform rounding! rather, fill remaining digits.
        for (char& c : dst.subspan(i)) {
          c = '0';
        }
        return {dst, k};
      }

      size_t d = 0;
      if (mant >= scale8) {
        mant.sub(scale8);
        d += 8;
      }
      if (mant >= scale4) {
        mant.sub(scale4);
        d += 4;
      }
      if (mant >= scale2) {
        mant.sub(scale2);
        d += 2;
      }
      if (mant >= scale) {
        mant.sub(scale);
        d += 1;
      }
      EMIO_Z_DEV_ASSERT(mant < scale);
      EMIO_Z_DEV_ASSERT(d < 10);
      dst[i] = static_cast<char>('0' + d);
      mant.mul_small(10);
    }
  }

  // rounding up if we stop in the middle of digits
  // if the following digits are exactly 5000..., check the prior digit and try to
  // round to even (i.e., avoid rounding up when the prior digit is even).
  const auto order = mant <=> (scale.mul_small(5));
  if (order == std::strong_ordering::greater ||
      (order == std::strong_ordering::equal && len > 0 && (dst[len - 1] & 1) == 1)) {
    // if rounding up changes the length, the exponent should also change.
    // but we've been requested a fixed number of digits, so do not alter the buffer...
    if (std::optional<char> c = round_up(dst.subspan(0, len))) {
      k += 1;
      // ...unless we've been requested the fixed precision instead.
      // we also need to check that, if the original buffer was empty,
      // the additional digit can only be added when `k == limit` (edge case).
      if (k > -number_of_digits) {
        if (len == 0) {
          dst = buf.get_write_area_of(1).value();
        }

        if (len != 0 && len < dst.size()) {
          return {};
        }

        if (len < dst.size()) {
          dst[len] = *c;
          len += 1;
        }
      }
    }
  }
  return {dst.subspan(0, len), k};
}

inline constexpr format_fp_result_t format_shortest(const finite_result_t& dec, emio::buffer& buf) noexcept {
  // the number `v` to format is known to be:
  // - equal to `mant * 2^exp`;
  // - preceded by `(mant - 2 * minus) * 2^exp` in the original type; and
  // - followed by `(mant + 2 * plus) * 2^exp` in the original type.
  //
  // obviously, `minus` and `plus` cannot be zero. (for infinities, we use out-of-range values.)
  // also we assume that at least one digit is generated, i.e., `mant` cannot be zero too.
  //
  // this also means that any number between `low = (mant - minus) * 2^exp` and
  // `high = (mant + plus) * 2^exp` will map to this exact floating point number,
  // with bounds included when the original mantissa was even (i.e., `!mant_was_odd`).
  EMIO_Z_DEV_ASSERT(dec.mant > 0);
  EMIO_Z_DEV_ASSERT(dec.minus > 0);
  EMIO_Z_DEV_ASSERT(dec.plus > 0);
  //  EMIO_Z_DEV_ASSERT(buf.() >= MAX_SIG_DIGITS);

  // `a.cmp(&b) < rounding` is `if d.inclusive {a <= b} else {a < b}`
  const auto rounding = [&](std::strong_ordering ordering) {
    if (dec.inclusive) {
      return ordering <= 0;  // NOLINT(modernize-use-nullptr): false positive
    }
    return ordering < 0;  // NOLINT(modernize-use-nullptr): false positive
  };

  // estimate `k_0` from original inputs satisfying `10^(k_0-1) < high <= 10^(k_0+1)`.
  // the tight bound `k` satisfying `10^(k-1) < high <= 10^k` is calculated later.
  int16_t k = estimate_scaling_factor(dec.mant + dec.plus, dec.exp);

  // convert `{mant, plus, minus} * 2^exp` into the fractional form so that:
  // - `v = mant / scale`
  // - `low = (mant - minus) / scale`
  // - `high = (mant + plus) / scale`
  auto mant = bignum(dec.mant);
  auto minus = bignum(dec.minus);
  auto plus = bignum(dec.plus);
  auto scale = bignum(1U);

  size_t s2 = 0;
  size_t s5 = 0;
  size_t m2 = 0;
  size_t m5 = 0;

  if (dec.exp < 0) {
    s2 = static_cast<size_t>(-dec.exp);
  } else {
    m2 += static_cast<size_t>(dec.exp);
  }

  // divide `mant` by `10^k`. now `scale / 10 < mant + plus <= scale * 10`.
  if (k >= 0) {
    s2 += static_cast<size_t>(k);
    s5 += static_cast<size_t>(k);
  } else {
    m2 += static_cast<size_t>(-k);
    m5 += static_cast<size_t>(-k);
  }

  scale.mul_pow5(s5);
  scale.mul_pow2(s2);

  mant.mul_pow5(m5);
  mant.mul_pow2(m2);
  minus.mul_pow5(m5);
  minus.mul_pow2(m2);
  plus.mul_pow5(m5);
  plus.mul_pow2(m2);

  // fixup when `mant + plus > scale` (or `>=`).
  // we are not actually modifying `scale`, since we can skip the initial multiplication instead.
  // now `scale < mant + plus <= scale * 10` and we are ready to generate digits.
  //
  // note that `d[0]` *can* be zero, when `scale - plus < mant < scale`.
  // in this case rounding-up condition (`up` below) will be triggered immediately.
  if (rounding(scale <=> (bignum{mant}.add(plus)))) {
    // equivalent to scaling `scale` by 10
    k += 1;
  } else {
    mant.mul_small(10);
    minus.mul_small(10);
    plus.mul_small(10);
  }

  // cache `(2, 4, 8) * scale` for digit generation.
  bignum scale2 = scale;
  scale2.mul_pow2(1);
  bignum scale4 = scale;
  scale4.mul_pow2(2);
  bignum scale8 = scale;
  scale8.mul_pow2(3);

  auto dst = buf.get_write_area_of(std::numeric_limits<double>::max_digits10).value();

  bool down{};
  bool up{};
  size_t i{};
  while (true) {
    // invariants, where `d[0..n-1]` are digits generated so far:
    // - `v = mant / scale * 10^(k-n-1) + d[0..n-1] * 10^(k-n)`
    // - `v - low = minus / scale * 10^(k-n-1)`
    // - `high - v = plus / scale * 10^(k-n-1)`
    // - `(mant + plus) / scale <= 10` (thus `mant / scale < 10`)
    // where `d[i..j]` is a shorthand for `d[i] * 10^(j-i) + ... + d[j-1] * 10 + d[j]`.

    // generate one digit: `d[n] = floor(mant / scale) < 10`.
    size_t d = 0;
    if (mant >= scale8) {
      mant.sub(scale8);
      d += 8;
    }
    if (mant >= scale4) {
      mant.sub(scale4);
      d += 4;
    }
    if (mant >= scale2) {
      mant.sub(scale2);
      d += 2;
    }
    if (mant >= scale) {
      mant.sub(scale);
      d += 1;
    }
    EMIO_Z_DEV_ASSERT(mant < scale);
    EMIO_Z_DEV_ASSERT(d < 10);
    dst[i] = static_cast<char>('0' + d);
    i += 1;

    // this is a simplified description of the modified Dragon algorithm.
    // many intermediate derivations and completeness arguments are omitted for convenience.
    //
    // start with modified invariants, as we've updated `n`:
    // - `v = mant / scale * 10^(k-n) + d[0..n-1] * 10^(k-n)`
    // - `v - low = minus / scale * 10^(k-n)`
    // - `high - v = plus / scale * 10^(k-n)`
    //
    // assume that `d[0..n-1]` is the shortest representation between `low` and `high`,
    // i.e., `d[0..n-1]` satisfies both of the following but `d[0..n-2]` doesn't:
    // - `low < d[0..n-1] * 10^(k-n) < high` (bijectivity: digits round to `v`); and
    // - `abs(v / 10^(k-n) - d[0..n-1]) <= 1/2` (the last digit is correct).
    //
    // the second condition simplifies to `2 * mant <= scale`.
    // solving invariants in terms of `mant`, `low` and `high` yields
    // a simpler version of the first condition: `-plus < mant < minus`.
    // since `-plus < 0 <= mant`, we have the correct shortest representation
    // when `mant < minus` and `2 * mant <= scale`.
    // (the former becomes `mant <= minus` when the original mantissa is even.)
    //
    // when the second doesn't hold (`2 * mant > scale`), we need to increase the last digit.
    // this is enough for restoring that condition: we already know that
    // the digit generation guarantees `0 <= v / 10^(k-n) - d[0..n-1] < 1`.
    // in this case, the first condition becomes `-plus < mant - scale < minus`.
    // since `mant < scale` after the generation, we have `scale < mant + plus`.
    // (again, this becomes `scale <= mant + plus` when the original mantissa is even.)
    //
    // in short:
    // - stop and round `down` (keep digits as is) when `mant < minus` (or `<=`).
    // - stop and round `up` (increase the last digit) when `scale < mant + plus` (or `<=`).
    // - keep generating otherwise.
    down = rounding(mant <=> (minus));
    up = rounding(scale <=> (bignum{mant}.add(plus)));
    if (down || up) {
      // we have the shortest representation, proceed to the rounding
      break;
    }

    // restore the invariants.
    // this makes the algorithm always terminating: `minus` and `plus` always increases,
    // but `mant` is clipped modulo `scale` and `scale` is fixed.
    mant.mul_small(10);
    minus.mul_small(10);
    plus.mul_small(10);
  }

  // rounding up happens when
  // i) only the rounding-up condition was triggered, or
  // ii) both conditions were triggered and tie breaking prefers rounding up.
  if (up && (!down || mant.mul_pow2(1) >= scale)) {
    // if rounding up changes the length, the exponent should also change.
    // it seems that this condition is very hard to satisfy (possibly impossible),
    // but we are just being safe and consistent here.
    // SAFETY: we initialized that memory above.
    if (std::optional<char> c = round_up(dst.subspan(0, i))) {
      dst[i] = *c;
      i += 1;
      k += 1;
    }
  }
  return {dst.subspan(0, i), k};
}

}  // namespace emio::detail::format

namespace emio {

template <typename>
class formatter;

namespace detail::format {

//
// Write args.
//

inline constexpr result<void> write_padding_left(writer& wtr, format_specs& specs, size_t width) {
  if (specs.width == 0 || specs.width < static_cast<int>(width)) {
    specs.width = 0;
    return success;
  }
  int fill_width = specs.width - static_cast<int>(width);
  if (specs.align == alignment::left) {
    specs.width = fill_width;
    return success;
  }
  if (specs.align == alignment::center) {
    fill_width = fill_width / 2;
  }
  specs.width -= fill_width + static_cast<int>(width);
  return wtr.write_char_n(specs.fill, static_cast<size_t>(fill_width));
}

inline constexpr result<void> write_padding_right(writer& wtr, format_specs& specs) {
  if (specs.width == 0 || (specs.align != alignment::left && specs.align != alignment::center)) {
    return success;
  }
  return wtr.write_char_n(specs.fill, static_cast<size_t>(specs.width));
}

template <alignment DefaultAlign, typename Func>
constexpr result<void> write_padded(writer& wtr, format_specs& specs, size_t width, Func&& func) {
  if (specs.align == alignment::none) {
    specs.align = DefaultAlign;
  }
  EMIO_TRYV(write_padding_left(wtr, specs, width));
  EMIO_TRYV(func());
  return write_padding_right(wtr, specs);
}

inline constexpr result<std::pair<std::string_view, writer::write_int_options>> make_write_int_options(
    char spec_type) noexcept {
  using namespace alternate_form;

  std::string_view prefix;
  writer::write_int_options options{};

  switch (spec_type) {
  case no_type:
  case 'd':
    options.base = 10;
    break;
  case 'x':
    prefix = hex_lower;
    options = {.base = 16};
    break;
  case 'X':
    prefix = hex_upper;
    options = {.base = 16, .upper_case = true};
    break;
  case 'b':
    prefix = bin_lower;
    options.base = 2;
    break;
  case 'B':
    prefix = bin_upper;
    options.base = 2;
    break;
  case 'o':
    prefix = octal;
    options.base = 8;
    break;
  default:
    return err::invalid_format;
  }
  return std::pair{prefix, options};
}

inline constexpr result<char> try_write_sign(writer& wtr, const format_specs& specs, bool is_negative) {
  char sign_to_write = no_sign;
  if (is_negative) {
    sign_to_write = '-';
  } else if (specs.sign == '+' || specs.sign == ' ') {
    sign_to_write = specs.sign;
  }
  if (sign_to_write != no_sign && specs.zero_flag) {
    EMIO_TRYV(wtr.write_char(sign_to_write));
    return no_sign;
  }
  return sign_to_write;
}

inline constexpr result<std::string_view> try_write_prefix(writer& wtr, const format_specs& specs,
                                                           std::string_view prefix) {
  const bool write_prefix = specs.alternate_form && !prefix.empty();
  if (write_prefix && specs.zero_flag) {
    EMIO_TRYV(wtr.write_str(prefix));
    return "";
  }
  if (write_prefix) {
    return prefix;
  }
  return "";
}

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer& wtr, format_specs& specs, const Arg& arg) noexcept {
  if (specs.type == 'c') {
    return write_padded<alignment::left>(wtr, specs, 1, [&] {
      return wtr.write_char(static_cast<char>(arg));
    });
  }
  EMIO_TRY((auto [prefix, options]), make_write_int_options(specs.type));

  if (specs.type == 'o' && arg == 0) {
    prefix = "";
  }

  const auto abs_number = detail::to_absolute(arg);
  const bool is_negative = detail::is_negative(arg);
  const size_t num_digits = detail::get_number_of_digits(abs_number, options.base);

  EMIO_TRY(const char sign_to_write, try_write_sign(wtr, specs, is_negative));
  EMIO_TRY(const std::string_view prefix_to_write, try_write_prefix(wtr, specs, prefix));

  size_t total_width = num_digits;
  if (specs.alternate_form) {
    total_width += prefix.size();
  }
  if (is_negative || specs.sign == ' ' || specs.sign == '+') {
    total_width += 1;
  }

  return write_padded<alignment::right>(wtr, specs, total_width, [&, &options = options]() -> result<void> {
    const size_t area_size =
        num_digits + static_cast<size_t>(sign_to_write != no_sign) + static_cast<size_t>(prefix_to_write.size());
    EMIO_TRY(auto area, wtr.get_buffer().get_write_area_of(area_size));
    auto* it = area.data();
    if (sign_to_write != no_sign) {
      *it++ = sign_to_write;
    }
    if (!prefix_to_write.empty()) {
      it = copy_n(prefix_to_write.data(), prefix_to_write.size(), it);
    }
    write_number(abs_number, options.base, options.upper_case, it + detail::to_signed(num_digits));
    return success;
  });
}

inline constexpr result<void> write_non_finite(writer& wtr, bool upper_case, bool is_inf) {
  if (is_inf) {
    EMIO_TRYV(wtr.write_str(upper_case ? "INF" : "inf"));
  } else {
    EMIO_TRYV(wtr.write_str(upper_case ? "NAN" : "nan"));
  }
  return success;
}

// A floating-point presentation format.
enum class fp_format : uint8_t {
  general,  // General: exponent notation or fixed point based on magnitude.
  exp,      // Exponent notation with the default precision of 6, e.g. 1.2e-3.
  fixed,    // Fixed point with the default precision of 6, e.g. 0.0012.
  hex
};

struct fp_format_specs {
  int16_t precision;
  fp_format format;
  bool upper_case;
  bool showpoint;
};

inline constexpr fp_format_specs parse_fp_format_specs(const format_specs& specs) {
  constexpr int16_t default_precision = 6;

  // This spec is typically for general format.
  fp_format_specs fp_specs{
      .precision =
          specs.precision >= 0 || specs.type == no_type ? static_cast<int16_t>(specs.precision) : default_precision,
      .format = fp_format::general,
      .upper_case = specs.type == 'E' || specs.type == 'F' || specs.type == 'G',
      .showpoint = specs.alternate_form,
  };

  if (specs.type == 'e' || specs.type == 'E') {
    fp_specs.format = fp_format::exp;
    fp_specs.precision += 1;
    fp_specs.showpoint |= specs.precision != 0;
  } else if (specs.type == 'f' || specs.type == 'F') {
    fp_specs.format = fp_format::fixed;
    fp_specs.showpoint |= specs.precision != 0;
  } else if (specs.type == 'a' || specs.type == 'A') {
    fp_specs.format = fp_format::hex;
  }
  if (fp_specs.format != fp_format::fixed && fp_specs.precision == 0) {
    fp_specs.precision = 1;  // Calculate at least on significand.
  }
  return fp_specs;
}

inline constexpr char* write_significand(char* out, const char* significand, int significand_size, int integral_size,
                                         char decimal_point) {
  out = copy_n(significand, integral_size, out);
  if (decimal_point == 0) {
    return out;
  }
  *out++ = decimal_point;
  return copy_n(significand + integral_size, significand_size - integral_size, out);
}

inline constexpr char* write_exponent(char* it, int exp) {
  if (exp < 0) {
    *it++ = '-';
    exp = -exp;
  } else {
    *it++ = '+';
  }
  int cnt = 2;
  if (exp >= 100) {
    write_decimal(to_unsigned(exp), it + 3);
    return it;
  } else if (exp < 10) {
    *it++ = '0';
    cnt -= 1;
  }
  write_decimal(to_unsigned(exp), it + cnt);
  return it;
}

inline constexpr result<void> write_decimal(writer& wtr, format_specs& specs, fp_format_specs& fp_specs,
                                            bool is_negative, const format_fp_result_t& f) noexcept {
  const char* significand = f.digits.data();
  int significand_size = static_cast<int>(f.digits.size());
  const int output_exp = f.exp - 1;  // 0.1234 x 10^exp => 1.234 x 10^(exp-1)
  const int abs_output_exp = static_cast<uint16_t>(output_exp >= 0 ? output_exp : -output_exp);
  const bool has_sign = is_negative || specs.sign == ' ' || specs.sign == '+';

  if (fp_specs.format == fp_format::general && significand_size > 1) {
    // Remove trailing zeros.
    auto it = std::find_if(f.digits.rbegin(), f.digits.rend(), [](char c) {
      return c != '0';
    });
    significand_size -= static_cast<int>(it - f.digits.rbegin());
  }

  const auto use_exp_format = [=]() {
    if (fp_specs.format == fp_format::exp) {
      return true;
    }
    if (fp_specs.format != fp_format::general) {
      return false;
    }
    // Use the fixed notation if the exponent is in [exp_lower, exp_upper),
    // e.g. 0.0001 instead of 1e-04. Otherwise, use the exponent notation.
    constexpr int exp_lower = -4;
    constexpr int exp_upper = 16;
    return output_exp < exp_lower || output_exp >= (fp_specs.precision > 0 ? fp_specs.precision : exp_upper);
  };

  EMIO_TRY(const char sign_to_write, try_write_sign(wtr, specs, is_negative));

  int num_zeros = 0;
  char decimal_point = '.';
  size_t total_width = static_cast<uint32_t>(has_sign);
  size_t num_digits = to_unsigned(significand_size);

  if (use_exp_format()) {
    if (fp_specs.showpoint) {                             // Multiple significands or high precision.
      num_zeros = fp_specs.precision - significand_size;  // Trailing zeros after an zero only.
      if (num_zeros < 0) {
        num_zeros = 0;
      }
      num_digits += to_unsigned(num_zeros);
    } else if (significand_size == 1) {  // One significand.
      decimal_point = 0;
    }
    // The else part is general format with significand size less than the exponent.

    const int exp_digits = abs_output_exp >= 100 ? 3 : 2;
    num_digits += to_unsigned((decimal_point != 0 ? 1 : 0) + 2 /* sign + e */ + exp_digits);
    total_width += num_digits;

    return write_padded<alignment::right>(wtr, specs, total_width, [&]() -> result<void> {
      const size_t area_size = num_digits + static_cast<size_t>(sign_to_write != no_sign);
      EMIO_TRY(auto area, wtr.get_buffer().get_write_area_of(area_size));
      auto* it = area.data();
      if (sign_to_write != no_sign) {
        *it++ = sign_to_write;
      }

      it = write_significand(it, significand, significand_size, 1, decimal_point);
      it = fill_n(it, num_zeros, '0');
      *it++ = fp_specs.upper_case ? 'E' : 'e';
      write_exponent(it, output_exp);

      return success;
    });
  }

  int integral_size = 0;
  int num_zeros_2 = 0;

  if (output_exp < 0) {                                                   // Only fractional-part.
    num_digits += 2;                                                      // For zero + Decimal point.
    num_zeros = abs_output_exp - 1;                                       // Leading zeros after dot.
    if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#g}, 0.1) -> 0.100000 instead 0.1
      num_zeros_2 = fp_specs.precision - significand_size;
    }
  } else if ((output_exp + 1) >= significand_size) {  // Only integer-part (including zero).
    integral_size = significand_size;
    num_zeros = output_exp - significand_size + 1;  // Trailing zeros.
    if (fp_specs.showpoint) {                       // Significand is zero but fractional requested.
      if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#.4g}, 1) -> 1.000 instead of 1.
        num_zeros_2 = fp_specs.precision - significand_size - num_zeros;
      } else if (num_zeros == 0) {  // ({:f}, 0) or ({:.4f}, 1.23e-06) -> 0.000000 instead of 0
        num_zeros_2 = fp_specs.precision;
      }
      EMIO_Z_DEV_ASSERT(num_zeros >= 0);
      num_digits += 1;
    } else {  // Digit without zero
      decimal_point = 0;
    }
  } else {  // Both parts. Trailing zeros are part of significands.
    integral_size = output_exp + 1;
    num_digits += 1;                                                      // Decimal point.
    if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#g}, 1.2) -> 1.20000 instead 1.2
      num_zeros = fp_specs.precision - significand_size;
    }
    if (fp_specs.format == fp_format::fixed && significand_size > integral_size &&
        significand_size - integral_size < fp_specs.precision) {  // ({:.4}, 0.99999) -> 1.0000 instead of 1.00
      num_zeros = fp_specs.precision - (significand_size - integral_size);
    }
  }
  if (num_zeros < 0) {
    num_zeros = 0;
  }
  if (num_zeros_2 < 0) {
    num_zeros_2 = 0;
  }
  num_digits += static_cast<size_t>(num_zeros + num_zeros_2);
  total_width += num_digits;

  return write_padded<alignment::right>(wtr, specs, total_width, [&]() -> result<void> {
    const size_t area_size = num_digits + static_cast<size_t>(sign_to_write != no_sign);
    EMIO_TRY(auto area, wtr.get_buffer().get_write_area_of(area_size));
    auto* it = area.data();
    if (sign_to_write != no_sign) {
      *it++ = sign_to_write;
    }

    if (output_exp < 0) {
      *it++ = '0';
      if (decimal_point != 0) {
        *it++ = decimal_point;
        it = fill_n(it, num_zeros, '0');  // TODO: simplify fill_n/copy/copy/n + it
        it = copy_n(significand, significand_size, it);
        fill_n(it, num_zeros_2, '0');
      }
    } else if ((output_exp + 1) >= significand_size) {
      it = copy_n(significand, integral_size, it);
      if (num_zeros != 0) {
        it = fill_n(it, num_zeros, '0');
      }
      if (decimal_point != 0) {
        *it++ = '.';
        if (num_zeros_2 != 0) {
          fill_n(it, num_zeros_2, '0');
        }
      }
    } else {
      it = write_significand(it, significand, significand_size, integral_size, decimal_point);
      if (num_zeros != 0) {
        fill_n(it, num_zeros, '0');
      }
    }

    return success;
  });
}

inline constexpr std::array<char, 1> zero_digit{'0'};

inline constexpr format_fp_result_t format_decimal(buffer& buffer, const fp_format_specs& fp_specs,
                                                   const decode_result_t& decoded) {
  if (decoded.category == category::zero) {
    return format_fp_result_t{zero_digit, 1};
  }
  switch (fp_specs.format) {
  case fp_format::general:
    if (fp_specs.precision == no_precision) {
      return format_shortest(decoded.finite, buffer);
    }
    [[fallthrough]];
  case fp_format::exp:
    return format_exact(decoded.finite, buffer, format_exact_mode::significand_digits, fp_specs.precision);
  case fp_format::fixed: {
    auto res = format_exact(decoded.finite, buffer, format_exact_mode::decimal_point, fp_specs.precision);
    if (res.digits.empty()) {
      return format_fp_result_t{zero_digit, 1};
    }
    return res;
  }
  case fp_format::hex:
    std::terminate();
  }
  EMIO_Z_INTERNAL_UNREACHABLE;
}

inline constexpr result<void> format_and_write_decimal(writer& wtr, format_specs& specs,
                                                       const decode_result_t& decoded) noexcept {
  fp_format_specs fp_specs = parse_fp_format_specs(specs);

  if (decoded.category == category::infinity || decoded.category == category::nan) {
    if (specs.zero_flag) {  // Words aren't prefixed with zeros.
      specs.fill = ' ';
      specs.zero_flag = false;
    }
    EMIO_TRY(const char sign_to_write, try_write_sign(wtr, specs, decoded.negative));

    const size_t total_length = 3 + static_cast<uint32_t>(sign_to_write != no_sign);
    return write_padded<alignment::left>(wtr, specs, total_length, [&]() -> result<void> {
      if (sign_to_write != no_sign) {
        EMIO_TRYV(wtr.write_char(sign_to_write));
      }
      return write_non_finite(wtr, fp_specs.upper_case, decoded.category == category::infinity);
    });
  }

  emio::memory_buffer buf;
  const format_fp_result_t res = format_decimal(buf, fp_specs, decoded);
  return write_decimal(wtr, specs, fp_specs, decoded.negative, res);
}

template <typename Arg>
  requires(std::is_floating_point_v<Arg> && sizeof(Arg) <= sizeof(double))
constexpr result<void> write_arg(writer& wtr, format_specs& specs, const Arg& arg) noexcept {
  return format_and_write_decimal(wtr, specs, decode(arg));
}

inline constexpr result<void> write_arg(writer& wtr, format_specs& specs, const std::string_view arg) noexcept {
  if (specs.type != '?') {
    return write_padded<alignment::left>(wtr, specs, arg.size(), [&] {
      return wtr.write_str(arg);
    });
  }
  return write_padded<alignment::left>(wtr, specs, arg.size() + 2U, [&] {
    return wtr.write_str_escaped(arg);
  });
}

template <typename Arg>
  requires(std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer& wtr, format_specs& specs, const Arg arg) noexcept {
  // If a type other than None/c is specified, write out as integer instead of char.
  if (specs.type != no_type && specs.type != 'c' && specs.type != '?') {
    return write_arg(wtr, specs, static_cast<uint8_t>(arg));
  }
  if (specs.type != '?') {
    return write_padded<alignment::left>(wtr, specs, 1, [&] {
      return wtr.write_char(arg);
    });
  }
  return write_padded<alignment::left>(wtr, specs, 3, [&] {
    return wtr.write_char_escaped(arg);
  });
}

template <typename Arg>
  requires(std::is_same_v<Arg, void*> || std::is_same_v<Arg, std::nullptr_t>)
constexpr result<void> write_arg(writer& wtr, format_specs& specs, Arg arg) noexcept {
  specs.alternate_form = true;
  specs.type = 'x';
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): valid cast
  return write_arg(wtr, specs, reinterpret_cast<uintptr_t>(arg));
}

template <typename Arg>
  requires(std::is_same_v<Arg, bool>)
constexpr result<void> write_arg(writer& wtr, format_specs& specs, Arg arg) noexcept {
  // If a type other than None/s is specified, write out as 1/0 instead of true/false.
  if (specs.type != no_type && specs.type != 's') {
    return write_arg(wtr, specs, static_cast<uint8_t>(arg));
  }
  if (arg) {
    return write_padded<alignment::left>(wtr, specs, 4, [&] {
      return wtr.write_str("true");
    });
  }
  return write_padded<alignment::left>(wtr, specs, 5, [&] {
    return wtr.write_str("false");
  });
}

//
// Checks.
//

// specs is passed by reference instead as return type to reduce copying of big value (and code bloat)
inline constexpr result<void> validate_format_specs(reader& rdr, format_specs& specs) noexcept {
  EMIO_TRY(char c, rdr.read_char());
  if (c == '}') {  // Format end.
    return success;
  }
  if (c == '{') {  // No dynamic spec support.
    return err::invalid_format;
  }

  bool fill_aligned = false;
  {
    // Parse for alignment specifier.
    EMIO_TRY(const char c2, rdr.peek());
    if (c2 == '<' || c2 == '^' || c2 == '>') {
      if (c2 == '<') {
        specs.align = alignment::left;
      } else if (c2 == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      fill_aligned = true;
      specs.fill = c;
      rdr.pop();
      EMIO_TRY(c, rdr.read_char());
    } else if (c == '<' || c == '^' || c == '>') {
      if (c == '<') {
        specs.align = alignment::left;
      } else if (c == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      fill_aligned = true;
      EMIO_TRY(c, rdr.read_char());
    }
  }
  if (c == '+' || c == '-' || c == ' ') {  // Sign.
    specs.sign = c;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '0') {         // Zero flag.
    if (!fill_aligned) {  // If fill/align is used, the zero flag is ignored.
      specs.fill = '0';
      specs.align = alignment::right;
      specs.zero_flag = true;
    }
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    EMIO_TRY(const uint32_t width, rdr.parse_int<uint32_t>());
    if (width > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.width = static_cast<int32_t>(width);
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '.') {  // Precision.
    EMIO_TRY(const uint32_t precision, rdr.parse_int<uint32_t>());
    if (precision > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.precision = static_cast<int32_t>(precision);
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isalpha(c) || c == '?') {  // Type.
    specs.type = c;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '}') {  // Format end.
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> parse_format_specs(reader& rdr, format_specs& specs) noexcept {
  char c = rdr.read_char().assume_value();
  if (c == '}') {  // Format end.
    return success;
  }

  bool fill_aligned = false;
  {
    // Parse for alignment specifier.
    const char c2 = rdr.peek().assume_value();
    if (c2 == '<' || c2 == '^' || c2 == '>') {
      if (c2 == '<') {
        specs.align = alignment::left;
      } else if (c2 == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      fill_aligned = true;
      specs.fill = c;
      rdr.pop();
      c = rdr.read_char().assume_value();
    } else if (c == '<' || c == '^' || c == '>') {
      if (c == '<') {
        specs.align = alignment::left;
      } else if (c == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      fill_aligned = true;
      c = rdr.read_char().assume_value();
    }
  }
  if (c == '+' || c == '-' || c == ' ') {  // Sign.
    specs.sign = c;
    c = rdr.read_char().assume_value();
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    c = rdr.read_char().assume_value();
  }
  if (c == '0') {         // Zero flag.
    if (!fill_aligned) {  // Ignoreable.
      specs.fill = '0';
      specs.align = alignment::right;
      specs.zero_flag = true;
    }
    c = rdr.read_char().assume_value();
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    specs.width = static_cast<int32_t>(rdr.parse_int<uint32_t>().assume_value());
    c = rdr.read_char().assume_value();
  }
  if (c == '.') {  // Precision.
    specs.precision = static_cast<int32_t>(rdr.parse_int<uint32_t>().assume_value());
    c = rdr.read_char().assume_value();
  }
  if (detail::isalpha(c) || c == '?') {  // Type.
    specs.type = c;
    rdr.pop();  // rdr.read_char() in validate_format_spec;
  }
  return success;
}

inline constexpr result<void> check_integral_specs(const format_specs& specs) noexcept {
  if (specs.precision != no_precision) {
    return err::invalid_format;
  }
  switch (specs.type) {
  case no_type:
  case 'b':
  case 'B':
  case 'c':
  case 'd':
  case 'o':
  case 'O':
  case 'x':
  case 'X':
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_unsigned_specs(const format_specs& specs) noexcept {
  if (specs.sign == no_sign) {
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_bool_specs(const format_specs& specs) noexcept {
  if (specs.type != no_type && specs.type != 's') {
    return check_integral_specs(specs);
  }
  if (specs.precision != no_precision) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_char_specs(const format_specs& specs) noexcept {
  if (specs.type != no_type && specs.type != 'c' && specs.type != '?') {
    return check_integral_specs(specs);
  }
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag || specs.precision != no_precision) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_pointer_specs(const format_specs& specs) noexcept {
  if (specs.type != no_type && specs.type != 'p') {
    return err::invalid_format;
  }
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag || specs.precision != no_precision) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_floating_point_specs(const format_specs& specs) noexcept {
  if (specs.precision > 1100) {
    return err::invalid_format;
  }

  switch (specs.type) {
  case no_type:
  case 'f':
  case 'F':
  case 'e':
  case 'E':
  case 'g':
  case 'G':
    //  case 'a': Not supported yet.
    //  case 'A':
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_string_view_specs(const format_specs& specs) noexcept {
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag || specs.precision != no_precision ||
      (specs.type != no_type && specs.type != 's' && specs.type != '?')) {
    return err::invalid_format;
  }
  return success;
}

// Specifies if T has an enabled formatter specialization.
template <typename Arg>
inline constexpr bool has_formatter_v = std::is_constructible_v<formatter<Arg>>;

template <typename T>
concept has_validate_function_v = requires {
                                    { formatter<T>::validate(std::declval<reader&>()) } -> std::same_as<result<void>>;
                                  };

template <typename T>
concept has_any_validate_function_v =
    requires { &formatter<T>::validate; } || std::is_member_function_pointer_v<decltype(&formatter<T>::validate)> ||
    requires { std::declval<formatter<T>>().validate(std::declval<reader&>()); };

template <typename Arg>
constexpr result<void> validate_for(reader& format_is) noexcept {
  // Check if a formatter exist and a correct validate method is implemented. If not, use the parse method.
  if constexpr (has_formatter_v<Arg>) {
    if constexpr (has_validate_function_v<Arg>) {
      return formatter<Arg>::validate(format_is);
    } else {
      static_assert(!has_any_validate_function_v<Arg>,
                    "Formatter seems to have a validate property which doesn't fit the desired signature.");
      return formatter<Arg>{}.parse(format_is);
    }
  } else {
    static_assert(has_formatter_v<Arg>,
                  "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
    return err::invalid_format;
  }
}

template <typename T>
inline constexpr bool is_core_type_v =
    std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, std::nullptr_t> ||
    std::is_same_v<T, void*> || std::is_same_v<T, std::string_view>;

template <input_validation FormatStringValidation, typename T>
concept formatter_parse_supports_format_string_validation =
    requires(T formatter) { formatter.template parse<FormatStringValidation>(std::declval<reader>()); };

template <input_validation FormatStringValidation, typename T>
inline constexpr result<void> invoke_formatter_parse(T& formatter, reader& format_is) noexcept {
  if constexpr (formatter_parse_supports_format_string_validation<FormatStringValidation, T>) {
    return formatter.template parse<FormatStringValidation>(format_is);
  } else {
    return formatter.parse(format_is);
  }
}

template <typename T>
concept has_format_as = requires(T arg) { format_as(arg); };

template <typename T>
using format_as_return_t = decltype(format_as(std::declval<T>()));

// To reduce code bloat, similar types are unified to a general one.
template <typename T>
struct unified_type;

template <typename T>
struct unified_type {
  using type = const T&;
};

template <typename T>
  requires(!std::is_integral_v<T> && !std::is_same_v<T, std::nullptr_t> && std::is_constructible_v<std::string_view, T>)
struct unified_type<T> {
  using type = std::string_view;
};

template <typename T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
struct unified_type<T> {
  using type = std::conditional_t<num_bits<T>() <= 32, int32_t, int64_t>;
};

template <typename T>
  requires(std::is_floating_point_v<T> && sizeof(T) <= sizeof(double))
struct unified_type<T> {
  using type = double;
};

template <typename T>
  requires(std::is_same_v<T, char> || std::is_same_v<T, bool> || std::is_same_v<T, void*> ||
           std::is_same_v<T, std::nullptr_t>)
struct unified_type<T> {
  using type = T;
};

template <typename T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
struct unified_type<T> {
  using type = std::conditional_t<num_bits<T>() <= 32, uint32_t, uint64_t>;
};

template <typename T>
using unified_type_t = typename unified_type<T>::type;

}  // namespace detail::format
}  // namespace emio

namespace emio {

/**
 * Checks if a type is formattable.
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool is_formattable_v = detail::format::has_formatter_v<std::remove_cvref_t<T>>;

/**
 * Class template that defines formatting rules for a given type.
 * @note This class definition is just a mock-up. See other template specialization for a concrete formatting.
 * @tparam T The type to format.
 */
template <typename T>
class formatter {
 public:
  // Not constructable because this is just a minimal example how to write a custom formatter.
  formatter() = delete;

  /**
   * Optional static function to validate the format spec for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param rdr The format reader.
   * @return Success if the format spec is valid.
   */
  static constexpr result<void> validate(reader& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param rdr The format reader.
   * @return Success if the format spec is valid and could be parsed.
   */
  constexpr result<void> parse(reader& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to format the object of this type according to the parsed format specs.
   * @param wtr The output writer.
   * @param arg The argument to format.
   * @return Success if the formatting could be done.
   */
  constexpr result<void> format(writer& wtr, const T& arg) const noexcept {
    return wtr.write_int(sizeof(arg));
  }
};

/**
 * Formatter for most common unambiguity types.
 * This includes:
 * - boolean
 * - char
 * - string_view
 * - void* / nullptr
 * - integral, floating-point types
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_core_type_v<T>)
class formatter<T> {
 public:
  static constexpr result<void> validate(reader& rdr) noexcept {
    detail::format::format_specs specs{};
    EMIO_TRYV(validate_format_specs(rdr, specs));
    if constexpr (std::is_same_v<T, bool>) {
      EMIO_TRYV(check_bool_specs(specs));
    } else if constexpr (std::is_same_v<T, char>) {
      EMIO_TRYV(check_char_specs(specs));
    } else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_same_v<T, void*>) {
      EMIO_TRYV(check_pointer_specs(specs));
    } else if constexpr (std::is_integral_v<T>) {
      EMIO_TRYV(check_integral_specs(specs));
      if constexpr (std::is_unsigned_v<T>) {
        EMIO_TRYV(check_unsigned_specs(specs));
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      EMIO_TRYV(check_floating_point_specs(specs));
    } else if constexpr (std::is_constructible_v<std::string_view, T>) {
      EMIO_TRYV(check_string_view_specs(specs));
    } else {
      static_assert(detail::always_false_v<T>, "Unknown core type!");
    }
    return success;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    return detail::format::parse_format_specs(rdr, specs_);
  }

  constexpr result<void> format(writer& wtr, const T& arg) const noexcept {
    auto specs = specs_;  // Copy spec because format could be called multiple times (e.g. ranges).
    return write_arg(wtr, specs, arg);
  }

  /**
   * Enables or disables the debug output format.
   * @note Used e.g. from range formatter.
   * @param enabled Flag to enable or disable the debug output.
   */
  constexpr void set_debug_format(bool enabled) noexcept
    requires(std::is_same_v<T, char> || std::is_same_v<T, std::string_view>)
  {
    if (enabled) {
      specs_.type = '?';
    } else {
      specs_.type = detail::format::no_type;
    }
  }

  /**
   * Sets the width.
   * @note Used e.g. from dynamic spec formatter.
   * @param width The width.
   */
  constexpr void set_width(int32_t width) noexcept {
    specs_.width = std::max<int32_t>(0, width);
  }

  /**
   * Sets the precision.
   * @note Used e.g. from dynamic spec formatter.
   * @param precision The precision.
   */
  constexpr void set_precision(int32_t precision) noexcept {
    specs_.precision = std::max<int32_t>(0, precision);
  }

 private:
  detail::format::format_specs specs_{};
};

/**
 * Formatter for any type which could be represented as a core type. E.g. string -> string_view.
 * @tparam T The unscoped enum type.
 */
template <typename T>
  requires(!detail::format::is_core_type_v<T> && detail::format::is_core_type_v<detail::format::unified_type_t<T>>)
class formatter<T> : public formatter<detail::format::unified_type_t<T>> {};

/**
 * Formatter for unscoped enum types to there underlying type.
 * @tparam T The unscoped enum type.
 */
template <typename T>
  requires(std::is_enum_v<T> && std::is_convertible_v<T, std::underlying_type_t<T>>)
class formatter<T> : public formatter<std::underlying_type_t<T>> {
 public:
  constexpr result<void> format(writer& wtr, const T& arg) const noexcept {
    return formatter<std::underlying_type_t<T>>::format(wtr, static_cast<std::underlying_type_t<T>>(arg));
  }
};

/**
 * Formatter for types which can formatted with a format_as function when using ADL.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::has_format_as<T>)
class formatter<T> : public formatter<detail::format::format_as_return_t<T>> {
 public:
  constexpr result<void> format(writer& wtr, const T& arg) const noexcept {
    return formatter<detail::format::format_as_return_t<T>>::format(wtr, format_as(arg));
  }
};

namespace detail {

template <typename T>
struct format_spec_with_value;

}

/**
 * Struct to dynamically specify width and precision.
 */
struct format_spec {
  /// Constant which indicates that the spec should not overwrite existing spec defined in the format string.
  static constexpr int32_t not_defined = -std::numeric_limits<int32_t>::max();

  /// The width.
  int32_t width{not_defined};
  /// The precision.
  int32_t precision{not_defined};

  /**
   * Returns an object that holds the value and the dynamic specification as reference.
   *
   * @note The object uses reference semantics and does not extend the lifetime of the held objects. It is the
   * programmer's responsibility to ensure that value outlive the return value. Usually, the result is only used as
   * argument to a formatting function.
   *
   * @param value The value to format.
   * @return Internal type.
   */
  template <typename T>
  [[nodiscard]] constexpr detail::format_spec_with_value<T> with(const T& value) const noexcept;
};

namespace detail {

/**
 * Struct holding the format spec and value.
 */
template <typename T>
struct format_spec_with_value {
  const format_spec& spec;
  const T& value;
};

}  // namespace detail

template <typename T>
[[nodiscard]] constexpr detail::format_spec_with_value<T> format_spec::with(const T& value) const noexcept {
  return {*this, value};
}

/**
 * Formatter for types whose format specification is dynamically defined.
 * @tparam T The underlying type.
 */
template <typename T>
class formatter<detail::format_spec_with_value<T>> {
 public:
  static constexpr result<void> validate(reader& rdr) noexcept {
    return formatter<T>::validate(rdr);
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    return underlying_.parse(rdr);
  }

  constexpr result<void> format(writer& wtr, const detail::format_spec_with_value<T>& arg) noexcept {
    overwrite_spec(arg.spec);
    return underlying_.format(wtr, arg.value);
  }

 private:
  template <typename F>
    requires requires(F x) {
               x.set_width(1);
               x.set_precision(1);
             }
  static constexpr F& get_core_formatter(F& formatter) noexcept {
    return formatter;
  }

  template <typename F>
    requires requires(F x) { x.underlying(); }
  static constexpr auto& get_core_formatter(F& formatter) noexcept {
    return get_core_formatter(formatter.underlying());
  }

  constexpr void overwrite_spec(const format_spec& spec) noexcept {
    // Overwrite the spec of the core formatter if they are dynamically defined.
    auto& f = get_core_formatter(underlying_);
    if (spec.width != format_spec::not_defined) {
      f.set_width(spec.width);
    }
    if (spec.precision != format_spec::not_defined) {
      f.set_precision(spec.precision);
    }
  }

  formatter<T> underlying_{};
};

}  // namespace emio

namespace emio::detail::format {

/**
 * Type erased format argument just for format string validation.
 */
class format_validation_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit format_validation_arg(std::type_identity<T> /*unused*/) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<unified_type_t<T>>*>(&storage_));
  }

  format_validation_arg(const format_validation_arg&) = delete;
  format_validation_arg(format_validation_arg&&) = delete;
  format_validation_arg& operator=(const format_validation_arg&) = delete;
  format_validation_arg& operator=(format_validation_arg&&) = delete;
  // No destructor & delete call to concept_t because model_t holds only a reference.
  ~format_validation_arg() = default;

  result<void> validate(reader& format_is) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->validate(format_is);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> validate(reader& format_is) const noexcept = 0;

   protected:
    ~concept_t() = default;
  };

  template <typename T>
  class model_t final : public concept_t {
   public:
    explicit model_t() noexcept = default;
    model_t(const model_t&) = delete;
    model_t(model_t&&) = delete;
    model_t& operator=(const model_t&) = delete;
    model_t& operator=(model_t&&) = delete;

    result<void> validate(reader& format_is) const noexcept override {
      return validate_for<std::remove_cvref_t<T>>(format_is);
    }

   protected:
    ~model_t() = default;
  };

  std::aligned_storage_t<sizeof(model_t<int>)> storage_;
};

/**
 * Format arguments just for format string validation.
 */
class format_validation_args {
 public:
  format_validation_args(const format_validation_args&) = delete;
  format_validation_args(format_validation_args&&) = delete;
  format_validation_args& operator=(const format_validation_args&) = delete;
  format_validation_args& operator=(format_validation_args&&) = delete;
  ~format_validation_args() = default;

  [[nodiscard]] std::string_view get_format_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const format_validation_arg> get_args() const noexcept {
    return args_;
  }

 protected:
  format_validation_args(std::string_view format_str, std::span<const format_validation_arg> args)
      : format_str_{format_str}, args_{args} {}

 private:
  std::string_view format_str_;
  std::span<const detail::format::format_validation_arg> args_;
};

/**
 * Format arguments storage just for format string validation.
 */
template <size_t NbrOfArgs>
class format_validation_args_storage : public format_validation_args {
 public:
  template <typename... Args>
  format_validation_args_storage(std::string_view str, const Args&... args)
      : format_validation_args{str, args_storage_}, args_storage_{format_validation_arg{args}...} {}

  format_validation_args_storage(const format_validation_args_storage&) = delete;
  format_validation_args_storage(format_validation_args_storage&&) = delete;
  format_validation_args_storage& operator=(const format_validation_args_storage&) = delete;
  format_validation_args_storage& operator=(format_validation_args_storage&&) = delete;
  ~format_validation_args_storage() = default;

 private:
  std::array<format_validation_arg, NbrOfArgs> args_storage_;
};

template <typename... Args>
format_validation_args_storage<sizeof...(Args)> make_format_validation_args(std::string_view format_str) {
  return {format_str, std::type_identity<Args>{}...};
}

/**
 * Type erased format argument for formatting.
 */
class format_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit format_arg(const T& value) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<unified_type_t<T>>*>(&storage_), value);
  }

  format_arg(const format_arg&) = delete;
  format_arg(format_arg&&) = delete;
  format_arg& operator=(const format_arg&) = delete;
  format_arg& operator=(format_arg&&) = delete;
  ~format_arg() = default;  // No destructor & delete call to concept_t because model_t holds only a reference.

  result<void> format(writer& wtr, reader& format_is) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->format(wtr, format_is);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> format(writer& wtr, reader& format_is) const noexcept = 0;

   protected:
    ~concept_t() = default;
  };

  template <typename T>
  class model_t final : public concept_t {
   public:
    explicit model_t(T value) noexcept : value_{value} {}

    model_t(const model_t&) = delete;
    model_t(model_t&&) = delete;
    model_t& operator=(const model_t&) = delete;
    model_t& operator=(model_t&&) = delete;

    result<void> format(writer& wtr, reader& format_is) const noexcept override {
      formatter<std::remove_cvref_t<T>> formatter;
      EMIO_TRYV(invoke_formatter_parse<input_validation::disabled>(formatter, format_is));
      return formatter.format(wtr, value_);
    }

   protected:
    ~model_t() = default;

   private:
    T value_;
  };

  std::aligned_storage_t<sizeof(model_t<std::string_view>)> storage_;
};

/**
 * Format arguments for formatting.
 */
class format_args {
 public:
  format_args(const format_args&) = delete;
  format_args(format_args&&) = delete;
  format_args& operator=(const format_args&) = delete;
  format_args& operator=(format_args&&) = delete;
  ~format_args() = default;

  result<std::string_view> get_format_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const format_arg> get_args() const noexcept {
    return args_;
  }

 protected:
  format_args(result<std::string_view> format_str, std::span<const format_arg> args)
      : format_str_{format_str}, args_{args} {}

 private:
  result<std::string_view> format_str_;
  std::span<const format_arg> args_;
};

/**
 * Format arguments storage for formatting.
 */
template <size_t NbrOfArgs>
class format_args_storage : public format_args {
 public:
  template <typename... Args>
  format_args_storage(result<std::string_view> str, const Args&... args)
      : format_args{str, args_storage_}, args_storage_{format_arg{args}...} {}

  format_args_storage(const format_args_storage&) = delete;
  format_args_storage(format_args_storage&&) = delete;
  format_args_storage& operator=(const format_args_storage&) = delete;
  format_args_storage& operator=(format_args_storage&&) = delete;
  ~format_args_storage() = default;

 private:
  std::array<format_arg, NbrOfArgs> args_storage_;
};

}  // namespace emio::detail::format

namespace emio::detail::format {

class format_parser final : public parser_base<input_validation::disabled> {
 public:
  constexpr explicit format_parser(writer& wtr, reader& format_rdr) noexcept
      : parser_base<input_validation::disabled>{format_rdr}, wtr_{wtr} {}

  format_parser(const format_parser&) = delete;
  format_parser(format_parser&&) = delete;
  format_parser& operator=(const format_parser&) = delete;
  format_parser& operator=(format_parser&&) = delete;
  constexpr ~format_parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char c) noexcept override {
    return wtr_.write_char(c);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> find_and_write_arg(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> find_and_write_arg(uint8_t arg_pos, const Arg& arg, const Args&... args) noexcept {
    if (arg_pos == 0) {
      return write_arg(arg);
    }
    return find_and_write_arg(arg_pos - 1, args...);
  }

 private:
  template <typename Arg>
  constexpr result<void> write_arg(const Arg& arg) noexcept {
    if constexpr (has_formatter_v<Arg>) {
      formatter<Arg> formatter;
      EMIO_TRYV(formatter.parse(this->format_rdr_));
      return formatter.format(wtr_, arg);
    } else {
      static_assert(has_formatter_v<Arg>,
                    "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
    }
  }

  writer& wtr_;
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
constexpr format_parser::~format_parser() noexcept = default;

class format_specs_checker final : public parser_base<input_validation::enabled> {
 public:
  using parser_base<input_validation::enabled>::parser_base;

  format_specs_checker(const format_specs_checker& other) = delete;
  format_specs_checker(format_specs_checker&& other) = delete;
  format_specs_checker& operator=(const format_specs_checker& other) = delete;
  format_specs_checker& operator=(format_specs_checker&& other) = delete;
  constexpr ~format_specs_checker() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char /*c*/) noexcept override {
    return success;
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  template <typename... Args>
    requires(sizeof...(Args) == 0)
  constexpr result<void> find_and_validate_arg(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> find_and_validate_arg(uint8_t arg_pos) noexcept {
    if (arg_pos == 0) {
      return validate_arg<Arg>();
    }
    return find_and_validate_arg<Args...>(arg_pos - 1);
  }

 private:
  template <typename Arg>
  constexpr result<void> validate_arg() noexcept {
    return validate_for<std::remove_cvref_t<Arg>>(this->format_rdr_);
  }
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
constexpr format_specs_checker::~format_specs_checker() noexcept = default;

[[nodiscard]] inline bool validate_format_string_fallback(const format_validation_args& args) noexcept {
  reader format_rdr{args.get_format_str()};
  format_specs_checker fh{format_rdr};
  bitset<128> matched{};
  const size_t arg_cnt = args.get_args().size();
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return false;
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (arg_cnt <= arg_nbr) {
      return false;
    }
    matched.set(arg_nbr);
    auto res = args.get_args()[arg_nbr].validate(format_rdr);
    if (!res) {
      return false;
    }
  }
  return matched.all_first(arg_cnt);
}

template <typename... Args>
[[nodiscard]] constexpr bool validate_format_string(std::string_view format_str) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    reader format_rdr{format_str};
    format_specs_checker fh{format_rdr};
    bitset<sizeof...(Args)> matched{};
    while (true) {
      uint8_t arg_nbr{detail::no_more_args};
      if (auto res = fh.parse(arg_nbr); !res) {
        return false;
      }
      if (arg_nbr == detail::no_more_args) {
        break;
      }
      if (matched.size() <= arg_nbr) {
        return false;
      }
      matched.set(arg_nbr);
      auto res = fh.template find_and_validate_arg<Args...>(arg_nbr);
      if (!res) {
        return false;
      }
    }
    return matched.all();
  } else {
    return validate_format_string_fallback(make_format_validation_args<Args...>(format_str));
  }
}

}  // namespace emio::detail::format

namespace emio::detail::format {

/**
 * This class represents a not yet validated format string, which has to be validated at runtime.
 */
class runtime_format_string {
 public:
  /**
   * Constructs an empty runtime format string.
   */
  constexpr runtime_format_string() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr runtime_format_string(std::string&&) = delete;
  constexpr runtime_format_string(std::nullptr_t) = delete;
  constexpr runtime_format_string(int) = delete;

  /**
   * Constructs the runtime format string from any suitable char sequence.
   * @param str The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  constexpr explicit runtime_format_string(const S& str) : str_{str} {}

  /**
   * Obtains a view over the runtime format string.
   * @return The view.
   */
  [[nodiscard]] constexpr std::string_view view() const noexcept {
    return str_;
  }

 private:
  std::string_view str_;
};

template <typename... Args>
class valid_format_string;

/**
 * This class represents a validated format string. The format string is either valid or not.
 * @note The validation happens at object construction.
 * @tparam Args The argument types to format.
 */
template <typename... Args>
class format_string {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval format_string(const S& s) noexcept {
    std::string_view str{s};
    if (validate_format_string<Args...>(str)) {
      str_ = str;
    } else {
      // Invalid format string detected. Stop compilation.
      std::terminate();
    }
  }

  /**
   * Constructs and validates a runtime format string at runtime.
   * @param s The runtime format string.
   */
  constexpr format_string(runtime_format_string s) noexcept {
    std::string_view str{s.view()};
    if (validate_format_string<Args...>(str)) {
      str_ = str;
    }
  }

  /**
   * Returns the validated format string as view.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::string_view> get() const noexcept {
    return str_;
  }

  /**
   * Returns format string as valid one.
   * @return The valid format string or invalid_format if the validation failed.
   */
  constexpr result<valid_format_string<Args...>> as_valid() const noexcept {
    if (str_.has_value()) {
      return valid_format_string<Args...>{valid, str_.assume_value()};
    }
    return err::invalid_format;
  }

 protected:
  static constexpr struct valid_t {
  } valid{};

  constexpr explicit format_string(valid_t /*unused*/, std::string_view s) noexcept : str_{s} {}

 private:
  result<std::string_view> str_{err::invalid_format};  ///< Validated format string.
};

/**
 * This class represents a validated format string. The format string can only be valid.
 * @tparam Args The argument types to format.
 */
template <typename... Args>
class valid_format_string : public format_string<Args...> {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval valid_format_string(const S& s) noexcept : format_string<Args...>{s} {}

  /**
   * Constructs and validates a format string at runtime.
   * @param s The format string.
   * @return The valid format string or invalid_format if the validation failed.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  static constexpr result<valid_format_string<Args...>> from(const S& s) noexcept {
    std::string_view str{s};
    if (!validate_format_string<Args...>(str)) {
      return err::invalid_format;
    }
    return valid_format_string{valid, str};
  }

 private:
  friend class format_string<Args...>;

  using valid_t = typename format_string<Args...>::valid_t;
  using format_string<Args...>::valid;

  constexpr explicit valid_format_string(valid_t /*unused*/, std::string_view s) noexcept
      : format_string<Args...>{valid, s} {}
};

}  // namespace emio::detail::format

namespace emio {

// Alias template types.
using runtime_format_string = detail::format::runtime_format_string;

template <typename... Args>
using format_string = detail::format::format_string<std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = detail::format::valid_format_string<std::type_identity_t<Args>...>;

inline constexpr runtime_format_string runtime(const std::string_view& s) noexcept {
  return runtime_format_string{s};
}

}  // namespace emio

namespace emio::detail::format {

// Non constexpr version.
inline result<void> vformat_to(buffer& buf, const format_args& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_format_str());
  reader format_rdr{str};
  writer wtr{buf};
  format_parser fh{wtr, format_rdr};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return res.assume_error();
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (auto res = args.get_args()[arg_nbr].format(wtr, format_rdr); !res) {
      return res.assume_error();
    }
  }
  return success;
}

// Constexpr version.
template <typename... Args>
constexpr result<void> format_to(buffer& buf, format_string<Args...> format_str, const Args&... args) noexcept {
  EMIO_TRY(std::string_view str, format_str.get());
  reader format_rdr{str};
  writer wtr{buf};
  format_parser fh{wtr, format_rdr};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return res.assume_error();
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (auto res = fh.find_and_write_arg(arg_nbr, args...); !res) {
      return res.assume_error();
    }
  }
  return success;
}

}  // namespace emio::detail::format

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <iterator>

namespace emio {

/**
 * This class provides the member types for an output iterator.
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
class truncating_iterator_base {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = typename std::iterator_traits<OutputIt>::value_type;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = void;

  /**
   * Default constructs a truncating iterator.
   */
  constexpr truncating_iterator_base() = default;

  /**
   * Constructs a truncating iterator with an output iterator to wrap and a given output limit.
   * @param out The output iterator.
   * @param limit The output limit.
   */
  constexpr truncating_iterator_base(OutputIt out, size_t limit)
      : out_{out}, limit_{static_cast<std::iter_difference_t<OutputIt>>(limit)} {}

  /**
   * Returns the actual output iterator.
   * @return The output iterator.
   */
  constexpr OutputIt out() const {
    return out_;
  }

  /**
   * Returns the count of actual passed elements to the wrapped output iterator.
   * @return The count.
   */
  [[nodiscard]] constexpr std::iter_difference_t<OutputIt> count() const noexcept {
    return count_;
  }

 protected:
  OutputIt out_{};
  std::iter_difference_t<OutputIt> limit_{};
  std::iter_difference_t<OutputIt> count_{};
};

/**
 * The truncating iterator is an output iterator which limits the amount of elements passed to an wrapped output
 * iterator.
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
class truncating_iterator;

// CTAD guide.
template <typename OutputIt>
truncating_iterator(OutputIt&&, size_t) -> truncating_iterator<std::decay_t<OutputIt>>;

/**
 * Template specification for a pure output-only iterator (e.g. the back_insert_iterator).
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
  requires(std::is_void_v<typename std::iterator_traits<OutputIt>::value_type>)
class truncating_iterator<OutputIt> : public truncating_iterator_base<OutputIt> {
 public:
  using truncating_iterator_base<OutputIt>::truncating_iterator_base;

  template <typename T>
  constexpr truncating_iterator& operator=(T val) {
    if (this->count_ < this->limit_) {
      ++this->count_;
      *this->out_++ = val;
    }
    return *this;
  }

  constexpr truncating_iterator& operator++() {
    return *this;
  }
  constexpr truncating_iterator& operator++(int) {
    return *this;
  }
  constexpr truncating_iterator& operator*() {
    return *this;
  }
};

/**
 * Template specification for any other output iterator.
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
  requires(!std::is_void_v<typename std::iterator_traits<OutputIt>::value_type>)
class truncating_iterator<OutputIt> : public truncating_iterator_base<OutputIt> {
 public:
  using value_type = typename truncating_iterator_base<OutputIt>::value_type;

  using truncating_iterator_base<OutputIt>::truncating_iterator_base;

  constexpr truncating_iterator& operator++() {
    if (this->count_ < this->limit_) {
      ++this->count_;
      ++this->out_;
    }
    return *this;
  }

  constexpr truncating_iterator operator++(int) {
    auto it = *this;
    ++*this;
    return it;
  }

  constexpr value_type& operator*() const {
    if (this->count_ < this->limit_) {
      return *this->out_;
    }
    return black_hole_;
  }

 private:
  mutable value_type black_hole_{};
};

namespace detail {

template <typename Iterator>
struct get_value_type;

template <typename Iterator>
struct get_value_type<truncating_iterator<Iterator>> : get_value_type<Iterator> {};

}  // namespace detail

}  // namespace emio

namespace emio {

/**
 * Provides access to the format string and the arguments to format.
 * @note This type should only be "constructed" via make_format_args(format_str, args...) and passed directly to an
 * formatting function.
 */
using format_args = detail::format::format_args;

// Alias type.
using format_args = detail::format::format_args;

/**
 * Returns an object that stores a format string with an array of all arguments to format.

 * @note The storage uses reference semantics and does not extend the lifetime of args. It is the programmer's
 * responsibility to ensure that args outlive the return value. Usually, the result is only used as argument to a
 * formatting function taking format_args by reference.

 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Internal type. Implicit convertible to format_args.
 */
template <typename... Args>
[[nodiscard]] detail::format::format_args_storage<sizeof...(Args)> make_format_args(format_string<Args...> format_str,
                                                                                    const Args&... args) noexcept {
  return {format_str.get(), args...};
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param args The format args with the format string.
 * @return The total number of characters in the formatted string or invalid_format if the format string validation
 * failed.
 */
inline result<size_t> vformatted_size(format_args&& args) noexcept {
  detail::counting_buffer buf{};
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  return buf.count();
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param format_str The format string
 * @param args The arguments to be formatted.
 * @return The total number of characters in the formatted string.
 */
template <typename... Args>
[[nodiscard]] constexpr size_t formatted_size(valid_format_string<Args...> format_str,
                                              const Args&... args) noexcept(detail::exceptions_disabled) {
  detail::counting_buffer buf{};
  detail::format::format_to(buf, format_str, args...).value();
  return buf.count();
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param format_str The format string
 * @param args The arguments to be formatted.
 * @return The total number of characters in the formatted string on success or invalid_format if the format string
 * validation failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_format_string> || std::is_same_v<T, format_string<Args...>>)
constexpr result<size_t> formatted_size(T format_str, const Args&... args) noexcept {
  detail::counting_buffer buf{};
  format_string<Args...> str{format_str};
  EMIO_TRYV(detail::format::format_to(buf, str, args...));
  return buf.count();
}

/**
 * Formats arguments according to the format string, and writes the result to the output buffer.
 * @param buf The output buffer.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
template <typename Buffer>
  requires(std::is_base_of_v<buffer, Buffer>)
result<void> vformat_to(Buffer& buf, const format_args& args) noexcept {
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the writer's buffer.
 * @param wrt The writer.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
inline result<void> vformat_to(writer& wrt, const format_args& args) noexcept {
  EMIO_TRYV(detail::format::vformat_to(wrt.get_buffer(), args));
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the output iterator.
 * @param out The output iterator.
 * @param args The format args with the format string.
 * @return The iterator past the end of the output range on success or EOF if the buffer is to small or invalid_format
 * if the format string validation failed.
 */
template <typename OutputIt, typename... Args>
  requires(std::output_iterator<OutputIt, char>)
constexpr result<OutputIt> vformat_to(OutputIt out, const format_args& args) noexcept {
  iterator_buffer buf{out};
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  return buf.out();
}

/**
 * Formats arguments according to the format string, and writes the result to the output buffer.
 * @param buf The output buffer.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
template <typename Buffer, typename... Args>
  requires(std::is_base_of_v<buffer, Buffer>)
constexpr result<void> format_to(Buffer& buf, format_string<Args...> format_str, const Args&... args) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the writer's buffer.
 * @param wrt The writer.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
template <typename... Args>
constexpr result<void> format_to(writer& wrt, format_string<Args...> format_str, const Args&... args) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(wrt.get_buffer(), format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(wrt.get_buffer(), make_format_args(format_str, args...)));
  }
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the output iterator.
 * @param out The output iterator.
 * @param args The format args with the format string.
 * @return The iterator past the end of the output range on success or EOF if the buffer is to small or invalid_format
 * if the format string validation failed.
 */
template <typename OutputIt, typename... Args>
  requires(std::output_iterator<OutputIt, char>)
constexpr result<OutputIt> format_to(OutputIt out, format_string<Args...> format_str, const Args&... args) noexcept {
  iterator_buffer buf{out};
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  return buf.out();
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param args The format args with the format string.
 * @return The string on success or invalid_format if the format string validation
 * failed.
 */
inline result<std::string> vformat(const format_args& args) noexcept {
  memory_buffer buf;
  if (auto res = detail::format::vformat_to(buf, args); !res) {
    return res.assume_error();
  }
  return buf.str();
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The string.
 */
template <typename... Args>
[[nodiscard]] std::string format(valid_format_string<Args...> format_str,
                                 const Args&... args) noexcept(detail::exceptions_disabled) {
  return vformat(make_format_args(format_str, args...)).value();  // Should never fail.
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The string on success or invalid_format if the format string validation
 * failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_format_string> || std::is_same_v<T, format_string<Args...>>)
result<std::string> format(T format_str, const Args&... args) noexcept {
  return vformat(make_format_args(format_str, args...));
}

/**
 * Return type of (v)format_to_n functions.
 * @tparam OutputIt The output iterator type.
 */
template <typename OutputIt>
struct format_to_n_result {
  OutputIt out;                           ///< The iterator past the end.
  std::iter_difference_t<OutputIt> size;  ///< The total number (not truncated) output size.
};

/**
 * Formats arguments according to the format string, and writes the result to the output iterator. At most n characters
 * are written.
 * @param out The output iterator.
 * @param n The maximum number of characters to be written to the buffer.
 * @param args The format args with the format string.
 * @return The format_to_n_result on success or invalid_format if the format string validation failed.
 */
template <typename OutputIt>
  requires(std::output_iterator<OutputIt, char>)
result<format_to_n_result<OutputIt>> vformat_to_n(OutputIt out, std::iter_difference_t<OutputIt> n,
                                                  const format_args& args) noexcept {
  truncating_iterator tout{out, static_cast<size_t>(n)};
  iterator_buffer buf{tout};
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  tout = buf.out();
  return format_to_n_result<OutputIt>{tout.out(), tout.count()};
}

/**
 * Formats arguments according to the format string, and writes the result to the output iterator. At most n characters
 * are written.
 * @param out The output iterator.
 * @param n The maximum number of characters to be written to the buffer.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The format_to_n_result on success or invalid_format if the format string validation failed.
 */
template <typename OutputIt, typename... Args>
  requires(std::output_iterator<OutputIt, char>)
constexpr result<format_to_n_result<OutputIt>> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n,
                                                           format_string<Args...> format_str,
                                                           const Args&... args) noexcept {
  truncating_iterator tout{out, static_cast<size_t>(n)};
  iterator_buffer buf{tout};
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  tout = buf.out();
  return format_to_n_result<OutputIt>{tout.out(), tout.count()};
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream.
 * @param file The file stream.
 * @param args The format args with the format string.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
inline result<void> vprint(std::FILE* file, const format_args& args) noexcept {
  if (file == nullptr) {
    return err::invalid_data;
  }

  file_buffer buf{file};
  EMIO_TRYV(vformat_to(buf, args));
  return buf.flush();
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream.
 * @param format_str The format string.
 * @param args The format args with the format string.
 */
template <typename... Args>
void print(valid_format_string<Args...> format_str, const Args&... args) {
  vprint(stdout, make_format_args(format_str, args...)).value();  // Should never fail.
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_format_string> || std::is_same_v<T, format_string<Args...>>)
result<void> print(T format_str, const Args&... args) {
  return vprint(stdout, make_format_args(format_str, args...));
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream.
 * @param file The file stream.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename... Args>
result<void> print(std::FILE* file, format_string<Args...> format_str, const Args&... args) {
  return vprint(file, make_format_args(format_str, args...));
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream with a new line at the
 * end.
 * @param file The file stream.
 * @param args The format args with the format string.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
inline result<void> vprintln(std::FILE* file, const format_args& args) noexcept {
  if (file == nullptr) {
    return err::invalid_data;
  }

  file_buffer buf{file};
  EMIO_TRYV(vformat_to(buf, args));
  EMIO_TRY(auto area, buf.get_write_area_of(1));
  area[0] = '\n';
  return buf.flush();
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream with a new line
 * at the end.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 */
template <typename... Args>
void println(valid_format_string<Args...> format_str, const Args&... args) {
  vprintln(stdout, make_format_args(format_str, args...)).value();  // Should never fail.
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream with a new line
 * at the end.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_format_string> || std::is_same_v<T, format_string<Args...>>)
result<void> println(T format_str, const Args&... args) {
  return vprintln(stdout, make_format_args(format_str, args...));
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream with a new line
 * at the end.
 * @param file The file stream.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename... Args>
result<void> println(std::FILE* file, format_string<Args...> format_str, const Args&... args) {
  return vprintln(file, make_format_args(format_str, args...));
}

}  // namespace emio

//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp
//
// Additional licence for this file:
// Copyright (c) 2018 - present, Remotion (Igor Schulz)

//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace emio::detail::format {

// For range.

using std::begin;
using std::data;
using std::end;
using std::size;

template <typename T>
concept advanceable = requires(T x) { ++x; };

template <typename T>
concept is_iterable = std::is_array_v<T> || requires(T x) {
                                              { begin(x) } -> advanceable;
                                              requires !std::is_same_v<decltype(*begin(x)), void>;
                                              { static_cast<bool>(begin(x) != end(x)) };
                                            };

template <typename T>
using element_type_t = std::remove_cvref_t<decltype(*begin(std::declval<std::remove_reference_t<T>&>()))>;

template <typename T>
concept is_map = requires(T x) { typename T::mapped_type; };

template <typename T>
concept is_set = requires(T x) { typename T::key_type; };

template <typename T>
concept is_string_like = std::is_constructible_v<std::string_view, T>;

template <typename T>
concept is_valid_range = is_iterable<T> && !
is_string_like<T>&& is_formattable_v<element_type_t<T>>;

template <typename T>
struct is_span : std::false_type {};

template <typename T, size_t N>
struct is_span<std::span<T, N>> : std::true_type {};

template <typename T>
concept is_contiguous_but_not_span =
    std::is_array_v<T> || requires(T x) {
                            requires !is_span<T>::value;
                            requires std::is_same_v<std::remove_cvref_t<decltype(*data(x))>, element_type_t<T>>;
                            { size(x) } -> std::same_as<size_t>;
                          };

struct ranges_specs {
  std::string_view opening_bracket{};
  std::string_view closing_bracket{};
  std::string_view separator{};
};

template <typename Formatter>
  requires requires(Formatter f) { f.set_debug_format(true); }
constexpr void maybe_set_debug_format(Formatter& f, bool set) noexcept {
  f.set_debug_format(set);
}

template <typename Formatter>
constexpr void maybe_set_debug_format(Formatter& /*unused*/, ...) noexcept {}

// For tuple like types.

using std::get;

// From https://stackoverflow.com/a/68444475/1611317
template <class T, std::size_t N>
concept has_tuple_element = requires(T t) {
                              typename std::tuple_element_t<N, std::remove_const_t<T>>;
                              { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
                            };

template <typename T, size_t... Ns>
constexpr auto has_tuple_element_unpack(std::index_sequence<Ns...> /*unused*/) {
  return (has_tuple_element<T, Ns> && ...);
}

template <class T>
concept is_tuple_like = !
std::is_reference_v<T>&& requires(T t) {
                           typename std::tuple_size<T>::type;
                           requires std::derived_from<std::tuple_size<T>,
                                                      std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
                         } && has_tuple_element_unpack<T>(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T, size_t... Ns>
constexpr auto is_formattable_unpack(std::index_sequence<Ns...> /*unused*/) {
  return (is_formattable_v<decltype(get<Ns>(std::declval<T&>()))> && ...);
}

template <typename T>
concept is_valid_tuple = !
is_valid_range<T>&& is_tuple_like<T>&& is_formattable_unpack<T>(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T, std::size_t... Ns>
auto get_tuple_formatters(std::index_sequence<Ns...> /*unused*/)
    -> std::tuple<formatter<std::remove_cvref_t<std::tuple_element_t<Ns, T>>>...>;

template <typename T>
using tuple_formatters = decltype(get_tuple_formatters<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

}  // namespace emio::detail::format

namespace emio {

/**
 * Formatter for ranges.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_valid_range<T> && !detail::format::is_contiguous_but_not_span<T>)
class formatter<T> {
 public:
  static constexpr result<void> validate(reader& rdr) noexcept {
    EMIO_TRY(char c, rdr.read_char());
    if (c == 'n') {
      EMIO_TRY(c, rdr.read_char());
    }
    if (c == '}') {
      return success;
    }
    if (c == ':') {
      return formatter<detail::format::element_type_t<T>>::validate(rdr);
    } else {
      return err::invalid_format;
    }
  }

  constexpr formatter() noexcept
    requires(!detail::format::is_map<T> && !detail::format::is_set<T>)
      : specs_{"[", "]", ", "} {}

  constexpr formatter() noexcept
    requires(detail::format::is_map<T>)
      : specs_{"{", "}", ", "} {
    underlying_.set_brackets({}, {});
    underlying_.set_separator(": ");
  }

  constexpr formatter() noexcept
    requires(detail::format::is_set<T> && !detail::format::is_map<T>)
      : specs_{"{", "}", ", "} {}

  constexpr void set_separator(std::string_view separator) noexcept {
    specs_.separator = separator;
  }

  constexpr void set_brackets(std::string_view opening_bracket, std::string_view closing_bracket) noexcept {
    specs_.opening_bracket = opening_bracket;
    specs_.closing_bracket = closing_bracket;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    char c = rdr.peek().assume_value();
    if (c == 'n') {
      set_brackets({}, {});
      rdr.pop();  // n
      c = rdr.peek().assume_value();
    }
    if (c == '}') {
      detail::format::maybe_set_debug_format(underlying_, true);
    } else {
      rdr.pop();  // :
    }
    return underlying_.parse(rdr);
  }

  constexpr result<void> format(writer& wtr, const T& arg) const noexcept {
    EMIO_TRYV(wtr.write_str(specs_.opening_bracket));

    using std::begin;
    using std::end;
    auto first = begin(arg);
    const auto last = end(arg);
    for (auto it = first; it != last; ++it) {
      if (it != first) {
        EMIO_TRYV(wtr.write_str(specs_.separator));
      }
      EMIO_TRYV(underlying_.format(wtr, *it));
    }
    EMIO_TRYV(wtr.write_str(specs_.closing_bracket));
    return success;
  }

  constexpr auto& underlying() noexcept {
    return underlying_;
  }

 private:
  formatter<detail::format::element_type_t<T>> underlying_{};
  detail::format::ranges_specs specs_{};
};

/**
 * Formatter for contiguous ranges.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_valid_range<T> && detail::format::is_contiguous_but_not_span<T>)
class formatter<T> : public formatter<std::span<const detail::format::element_type_t<T>>> {};

/**
 * Formatter for tuple like types.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_valid_tuple<T>)
class formatter<T> {
 public:
  constexpr formatter() : specs_{"(", ")", ", "} {}

  constexpr void set_separator(std::string_view separator) noexcept {
    specs_.separator = separator;
  }

  constexpr void set_brackets(std::string_view opening_bracket, std::string_view closing_bracket) noexcept {
    specs_.opening_bracket = opening_bracket;
    specs_.closing_bracket = closing_bracket;
  }

  static constexpr result<void> validate(reader& rdr) noexcept {
    EMIO_TRY(char c, rdr.read_char());
    if (c == 'n') {
      EMIO_TRY(c, rdr.read_char());
    }
    if (c == '}') {
      return success;
    }
    if (c == ':') {
      return validate_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), rdr);
    } else {
      return err::invalid_format;
    }
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    char c = rdr.peek().assume_value();
    if (c == 'n') {
      set_brackets({}, {});
      rdr.pop();  // n
      c = rdr.peek().assume_value();
    }
    bool set_debug = false;
    if (c == '}') {
      set_debug = true;
    } else {
      rdr.pop();  // :
    }
    return parse_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), rdr, set_debug);
  }

  constexpr result<void> format(writer& wtr, const T& args) const noexcept {
    EMIO_TRYV(wtr.write_str(specs_.opening_bracket));
    EMIO_TRYV(format_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), wtr, args));
    EMIO_TRYV(wtr.write_str(specs_.closing_bracket));
    return success;
  }

 private:
  template <size_t... Ns>
  static constexpr result<void> validate_for_each(std::index_sequence<Ns...> /*unused*/, reader& rdr) noexcept {
    size_t reader_pos = 0;
    result<void> res = success;
    const auto validate = [&reader_pos, &res]<typename U>(std::type_identity<U> /*unused*/, reader r /*copy!*/) {
      res = U::validate(r);
      if (res.has_error()) {
        return false;
      }
      if (reader_pos == 0) {
        reader_pos = r.pos();
      } else if (reader_pos != r.pos()) {
        res = err::invalid_format;
      }
      return res.has_value();
    };
    static_cast<void>(validate);  // Maybe unused warning.
    if ((validate(std::type_identity<std::tuple_element_t<Ns, detail::format::tuple_formatters<T>>>{}, rdr) && ...) &&
        reader_pos != 0) {
      rdr.pop(reader_pos);
      return success;
    }
    return res;
  }

  static constexpr result<void> validate_for_each(std::index_sequence<> /*unused*/, reader& /*rdr*/) noexcept {
    return err::invalid_format;
  }

  template <size_t... Ns>
  constexpr result<void> parse_for_each(std::index_sequence<Ns...> /*unused*/, reader& rdr,
                                        const bool set_debug) noexcept {
    using std::get;

    size_t reader_pos = 0;
    result<void> res = success;
    const auto parse = [&reader_pos, &res, set_debug](auto& f, reader r /*copy!*/) {
      detail::format::maybe_set_debug_format(f, set_debug);
      res = f.parse(r);
      reader_pos = r.pos();
      return res.has_value();
    };
    static_cast<void>(parse);  // Maybe unused warning.
    if ((parse(get<Ns>(formatters_), rdr) && ...)) {
      rdr.pop(reader_pos);
      return success;
    }
    return res;
  }

  constexpr result<void> parse_for_each(std::index_sequence<> /*unused*/, reader& rdr, const bool set_debug) noexcept {
    if (set_debug) {
      rdr.pop();  // }
      return success;
    }
    return err::invalid_format;
  }

  template <size_t N, size_t... Ns>
  constexpr result<void> format_for_each(std::index_sequence<N, Ns...> /*unused*/, writer& wtr,
                                         const T& args) const noexcept {
    using std::get;
    EMIO_TRYV(get<N>(formatters_).format(wtr, get<N>(args)));

    result<void> res = success;
    const auto format = [&res, &wtr, this](auto& f, const auto& arg) {
      res = wtr.write_str(specs_.separator);
      if (res.has_error()) {
        return false;
      }
      res = f.format(wtr, arg);
      return res.has_value();
    };
    static_cast<void>(format);  // Maybe unused warning.
    if ((format(get<Ns>(formatters_), get<Ns>(args)) && ...)) {
      return success;
    }
    return res;
  }

  constexpr result<void> format_for_each(std::index_sequence<> /*unused*/, writer& /*wtr*/,
                                         const T& /*args*/) const noexcept {
    return success;
  }

  detail::format::tuple_formatters<T> formatters_{};
  detail::format::ranges_specs specs_{};
};

}  // namespace emio

#endif  // EMIO_Z_MAIN_H
