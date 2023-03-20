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
#include <cstddef>
#include <string>

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

#if defined(__GNUC__) || defined(__GNUG__) || defined(_MSC_VER)
// Separate macro instead of std::is_constant_evaluated() because code will be optimized away even in debug if inlined.
#  define Y_EMIO_IS_CONST_EVAL __builtin_is_constant_evaluated()
#  define EMIO_Z_INTERNAL_UNREACHABLE __builtin_unreachable()
#else
#  define Y_EMIO_IS_CONST_EVAL std::is_constant_evaluated()
#  define EMIO_Z_INTERNAL_UNREACHABLE std::terminate()
#endif

}  // namespace emio::detail

namespace emio::detail {

/**
 * A constexpr basic_string with the bare minimum implementation.
 * @tparam Char The character type.
 */
template <typename Char>
class ct_basic_string {
 public:
  constexpr ct_basic_string() = default;

  ct_basic_string(const ct_basic_string&) = delete;
  ct_basic_string(ct_basic_string&&) = delete;
  ct_basic_string& operator=(const ct_basic_string&) = delete;
  ct_basic_string& operator=(ct_basic_string&&) = delete;

  constexpr ~ct_basic_string() noexcept {
    delete[] data_;  // NOLINT(cppcoreguidelines-owning-memory)
  }

  constexpr void resize(size_t new_size) noexcept {
    // Heavy pointer arithmetic because high level containers are not yet ready to use at constant evaluation.
    if (data_ == nullptr) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      data_ = new Char[new_size]{};  // NOLINT(cppcoreguidelines-owning-memory)
      capacity_ = new_size;
    } else if (capacity_ < new_size) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      Char* new_data = new Char[new_size]{};      // NOLINT(cppcoreguidelines-owning-memory)
      std::copy(data_, data_ + size_, new_data);  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      std::swap(new_data, data_);
      capacity_ = new_size;
      delete[] new_data;  // NOLINT(cppcoreguidelines-owning-memory)
    } else if (size_ < new_size) {
      std::fill(data_ + size_, data_ + new_size, Char{});  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
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
  Char* data_{};
  size_t size_{};
  size_t capacity_{};
};

/**
 * This union unites std::basic_string for runtime and ct_basic_string for compile-time.
 * @tparam Char The character type.
 */
template <typename Char>
union basic_string_union {
  constexpr basic_string_union() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      std::construct_at(&ct_str_);
    } else {
      std::construct_at(&str_);
    }
  }

  constexpr basic_string_union(const basic_string_union&) = delete;
  constexpr basic_string_union(basic_string_union&&) noexcept = delete;
  constexpr basic_string_union& operator=(const basic_string_union&) = delete;
  constexpr basic_string_union& operator=(basic_string_union&&) noexcept = default;

  constexpr ~basic_string_union() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      ct_str_.~ct_basic_string();
    } else {
      str_.~basic_string();
    }
  }

  [[nodiscard]] constexpr size_t capacity() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.capacity();
    } else {
      return str_.capacity();
    }
  }

  constexpr void resize(size_t size) noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      ct_str_.resize(size);
    } else {
      str_.resize(size);
    }
  }

  constexpr Char* data() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.data();
    } else {
      return str_.data();
    }
  }

  [[nodiscard]] constexpr const Char* data() const noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.data();
    } else {
      return str_.data();
    }
  }

  [[nodiscard]] constexpr size_t size() const noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.size();
    } else {
      return str_.size();
    }
  }

 private:
  std::basic_string<Char> str_;
  detail::ct_basic_string<Char> ct_str_;
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
  explicit bad_result_access(const char* msg) : logic_error{msg} {}
};

namespace detail {

#ifdef __EXCEPTIONS
inline constexpr bool exceptions_disabled = false;
#else
inline constexpr bool exceptions_disabled = true;
#endif

// Helper function to throw or terminate, depending on whether exceptions are globally enabled or not.
[[noreturn]] inline constexpr void throw_bad_result_access_or_terminate(const err error) noexcept(exceptions_disabled) {
  // Use dummy check to suppress compiler warnings/errors of throwing/terminating in constexpr.
  if (error != err{} || error == err{}) {
#ifdef __EXCEPTIONS
    throw bad_result_access{to_string(error).data()};
#else
    std::terminate();
#endif
  }
  EMIO_Z_INTERNAL_UNREACHABLE;
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
  constexpr result(err error) : error_{error} {}

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
   * Returns a pointer to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  constexpr std::remove_reference_t<Value>* operator->() noexcept {
    return &*value_;
  }

  /**
   * Returns a const pointer to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  constexpr const std::remove_reference_t<Value>* operator->() const noexcept {
    return &*value_;
  }

  /**
   * Returns a reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value& operator*() & noexcept {
    return *value_;
  }

  /**
   * Returns a const reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value& operator*() const& noexcept {
    return *value_;
  }

  /**
   * Returns a rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value&& operator*() && noexcept {
    return std::move(*value_);
  }

  /**
   * Returns a const rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value&& operator*() const&& noexcept {
    return std::move(*value_);
  }

  /**
   * Returns a reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value& assume_value() & noexcept {
    return *value_;
  }

  /**
   * Returns a const reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value& assume_value() const& noexcept {
    return *value_;
  }

  /**
   * Returns a rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr Value&& assume_value() && noexcept {
    return std::move(*value_);
  }

  /**
   * Returns a const rvalue reference to the value.
   * @note The behavior is undefined if this->has_value() is false.
   * @return The value.
   */
  [[nodiscard]] constexpr const Value&& assume_value() const&& noexcept {
    return std::move(*value_);
  }

  /**
   * Returns a reference to the value.
   * @return The value.
   */
  constexpr Value& value() & noexcept(detail::exceptions_disabled) {
    if (has_value()) [[likely]] {
      return *value_;
    }
    detail::throw_bad_result_access_or_terminate(error_);
  }

  /**
   * Returns a const reference to the value.
   * @return The value.
   */
  // NOLINTNEXTLINE(modernize-use-nodiscard): access check
  constexpr const Value& value() const& noexcept(detail::exceptions_disabled) {
    if (has_value()) [[likely]] {
      return *value_;
    }
    detail::throw_bad_result_access_or_terminate(error_);
  }

  /**
   * Returns a rvalue reference to the value.
   * @return The value.
   */
  constexpr Value&& value() && noexcept(detail::exceptions_disabled) {
    if (has_value()) [[likely]] {
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
    if (has_value()) [[likely]] {
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
  constexpr result(err error) : error_{error} {}

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
  constexpr void assume_value() const noexcept {
    // Nothing.
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
 * This class provides the basic API and functionality for receiving a contiguous memory region to write into.
 * @note Use a specific subclass for a concrete instantiation.
 * @tparam Char The character type.
 */
template <typename Char = char>
class buffer {
 public:
  buffer(const buffer& other) = delete;
  buffer(buffer&& other) = delete;
  buffer& operator=(const buffer& other) = delete;
  buffer& operator=(buffer&& other) = delete;
  constexpr virtual ~buffer() = default;

  /**
   * Returns a write area with the requested size on success.
   * @param size The size the write area should have.
   * @return The write area with the requested size on success or eof if no write area is available.
   */
  constexpr result<std::span<Char>> get_write_area_of(size_t size) noexcept {
    EMIO_TRY(const std::span<Char> area, get_write_area_of_max(size));
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
  constexpr result<std::span<Char>> get_write_area_of_max(size_t size) noexcept {
    const size_t remaining_capacity = area_.size() - used_;
    if (remaining_capacity >= size) {
      const std::span<char> area = area_.subspan(used_, size);
      used_ += size;
      return area;
    }
    EMIO_TRY(const std::span<Char> area, request_write_area(used_, size));
    used_ += area.size();
    return area;
  }

 protected:
  /**
   * Default constructs the buffer.
   */
  constexpr buffer() = default;

  /**
   * Requests a write area of the given size from a subclass.
   * @param used Already written characters into the current write area.
   * @param size The requested size of a new write area.
   * @return The write area with the requested size as maximum on success or eof if no write area is available.
   */
  constexpr virtual result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept {
    static_cast<void>(used);  // Keep params for documentation.
    static_cast<void>(size);
    return err::eof;
  }

  /**
   * Sets a new write area in the base class object to use.
   * @param area The new write area.
   */
  constexpr void set_write_area(const std::span<Char> area) noexcept {
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
  size_t used_{};
  std::span<Char> area_{};
};

/**
 * This class fulfills the buffer API by internal using a string object.
 * @tparam Char The character type.
 */
template <typename Char = char>
class string_buffer final : public buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with the basic capacity a string.
   */
  constexpr string_buffer() : string_buffer{0} {}

  /**
   * Constructs and initializes the buffer with the given capacity.
   * @param capacity The initial capacity of the string.
   */
  constexpr explicit string_buffer(const size_t capacity) {
    // Request at least the SBO capacity.
    static_cast<void>(request_write_area(0, std::max(data_.capacity(), capacity)));
  }

  constexpr string_buffer(const string_buffer&) = default;
  constexpr string_buffer(string_buffer&&) noexcept = default;
  constexpr string_buffer& operator=(const string_buffer&) = default;
  constexpr string_buffer& operator=(string_buffer&&) noexcept = default;
  constexpr ~string_buffer() override = default;

  /**
   * Obtains a view over the underlying string object.
   * @return The view.
   */
  [[nodiscard]] constexpr std::basic_string_view<Char> view() const noexcept {
    return {data_.data(), used_ + this->get_used_count()};
  }

  /**
   * Obtains a copy of the underlying string object.
   * @return The string.
   */
  [[nodiscard]] constexpr std::basic_string<Char> str() const {
    return std::string{view()};
  }

 protected:
  constexpr result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept override {
    const size_t new_size = data_.size() + size;
    data_.resize(new_size);
    used_ += used;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic): Performance in debug.
    const std::span<Char> area{data_.data() + used_, size};
    this->set_write_area(area);
    return area;
  }

 private:
  size_t used_{};
  detail::basic_string_union<Char> data_{};
};

/**
 * This class fulfills the buffer API by using a span over an contiguous range.
 * @tparam Char The character type.
 */
template <typename Char = char>
class span_buffer final : public buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with an empty span.
   */
  constexpr span_buffer() = default;

  /**
   * Constructs and initializes the buffer with the given span.
   * @param span The span.
   */
  constexpr explicit span_buffer(const std::span<Char> span) : span_{span} {
    this->set_write_area(span_);
  }

  constexpr span_buffer(const span_buffer&) = default;
  constexpr span_buffer(span_buffer&&) noexcept = default;
  constexpr span_buffer& operator=(const span_buffer&) = default;
  constexpr span_buffer& operator=(span_buffer&&) noexcept = default;
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
  [[nodiscard]] std::basic_string<Char> str() const {
    return std::string{view()};
  }

 private:
  std::span<Char> span_;
};

// Deduction guides.
template <typename T>
  requires std::is_constructible_v<std::span<char>, T>
span_buffer(T&&) -> span_buffer<char>;

template <typename T>
  requires std::is_constructible_v<std::span<char8_t>, T>
span_buffer(T&&) -> span_buffer<char8_t>;

template <typename T>
  requires std::is_constructible_v<std::span<char16_t>, T>
span_buffer(T&&) -> span_buffer<char16_t>;

template <typename T>
  requires std::is_constructible_v<std::span<char32_t>, T>
span_buffer(T&&) -> span_buffer<char32_t>;

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

template <typename Char, typename InputIt, typename OutputIt>
constexpr auto copy_str(InputIt it, InputIt end, OutputIt out) -> OutputIt {
  while (it != end) {
    *out++ = static_cast<Char>(*it++);
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
class iterator_buffer<Iterator> final : public buffer<detail::get_value_type_t<Iterator>> {
 private:
  using char_t = detail::get_value_type_t<Iterator>;

 public:
  /**
   * Constructs and initializes the buffer with the given output iterator.
   * @param it The output iterator.
   */
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
    it_ = detail::copy_str<char_t>(cache_.data(), cache_.data() + this->get_used_count(), it_);
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
  constexpr result<std::span<char_t>> request_write_area(const size_t /*used*/, const size_t size) noexcept override {
    flush();
    const std::span<char_t> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  Iterator it_;
  std::array<char_t, detail::internal_buffer_size> cache_;
};

/**
 * This class fulfills the buffer API by using an output pointer.
 * @tparam Iterator The output iterator type.
 */
template <typename OutputPtr>
  requires(std::input_or_output_iterator<OutputPtr*> &&
           std::output_iterator<OutputPtr*, detail::get_value_type_t<OutputPtr*>>)
class iterator_buffer<OutputPtr*> final : public buffer<detail::get_value_type_t<OutputPtr*>> {
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
class iterator_buffer<std::back_insert_iterator<Container>> final : public buffer<typename Container::value_type> {
 private:
  using char_t = typename Container::value_type;

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
  constexpr result<std::span<char_t>> request_write_area(const size_t used, const size_t size) noexcept override {
    const size_t new_size = container_.size() + size;
    container_.resize(new_size);
    used_ += used;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic): Performance in debug.
    const std::span<char_t> area{container_.data() + used_, new_size};
    this->set_write_area(area);
    return area.subspan(0, size);
  }

 private:
  size_t used_{};
  Container& container_;
};

template <typename Iterator>
iterator_buffer(Iterator&&) -> iterator_buffer<std::decay_t<Iterator>>;

namespace detail {

/**
 * A buffer that counts the number of code points written. Discards the output.
 * @tparam Char The used character type.
 */
template <typename Char>
class basic_counting_buffer final : public buffer<Char> {
 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): Can be left uninitialized.
  constexpr basic_counting_buffer() = default;
  constexpr basic_counting_buffer(const basic_counting_buffer&) = delete;
  constexpr basic_counting_buffer(basic_counting_buffer&&) noexcept = delete;
  constexpr basic_counting_buffer& operator=(const basic_counting_buffer&) = delete;
  constexpr basic_counting_buffer& operator=(basic_counting_buffer&&) noexcept = delete;
  constexpr ~basic_counting_buffer() override = default;

  /**
   * Calculates the number of code points that were written.
   * @return The number of code points.
   */
  [[nodiscard]] constexpr size_t count() const noexcept {
    return used_ + this->get_used_count();
  }

 protected:
  constexpr result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept override {
    used_ += used;
    const std::span<Char> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  size_t used_{};
  std::array<Char, detail::internal_buffer_size> cache_;
};

}  // namespace detail

}  // namespace emio

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>

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

template <typename T, typename OutputIt>
  requires(std::is_unsigned_v<T>)
constexpr OutputIt write_number(T abs_number, int base, bool upper, OutputIt next) {
  if (abs_number == 0) {
    *(--next) = '0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic): performance
    return next;
  }
  // Write number from right to left.
  for (; abs_number; abs_number /= static_cast<T>(base)) {
    const char c = digit_to_char(static_cast<int>(abs_number % static_cast<T>(base)), base, upper);
    *(--next) = c;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic): performance
  }
  return next;
}

inline constexpr size_t npos = std::string_view::npos;

template <typename Char>
constexpr std::basic_string_view<Char> unchecked_substr(const std::basic_string_view<Char>& str, size_t pos,
                                                        size_t n = npos) noexcept {
  const size_t rlen = std::min(n, str.length() - pos);
  return {str.data() + pos, rlen};
}

}  // namespace emio::detail

//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace emio::detail {

constexpr bool needs_escape(uint32_t cp) {
  return cp < 0x20 || cp >= 0x7f || cp == '\'' || cp == '"' || cp == '\\';
}

template <typename Char, typename OutputIt>
constexpr OutputIt write_escaped(std::basic_string_view<Char> sv, OutputIt out) {
  for (Char c : sv) {
    if (!needs_escape(static_cast<uint32_t>(c))) {
      *(out++) = c;
    } else {
      switch (c) {
      case '\n':
        *(out++) = '\\';
        *(out++) = 'n';
        break;
      case '\r':
        *(out++) = '\\';
        *(out++) = 'r';
        break;
      case '\t':
        *(out++) = '\\';
        *(out++) = 't';
        break;
      case '\\':
        *(out++) = '\\';
        *(out++) = '\\';
        break;
      case '\'':
        *(out++) = '\\';
        *(out++) = '\'';
        break;
      case '"':
        *(out++) = '\\';
        *(out++) = '"';
        break;
      default: {
        // Escape char zero filled like: \x05, \x0ABC, \x00ABCDEF
        *(out++) = '\\';
        *(out++) = 'x';
        const auto abs = detail::to_absolute(detail::to_unsigned(c));
        const size_t number_of_digits = count_digits<16>(abs);
        // Fill up with zeros.
        for (size_t i = 0; i < 2 * sizeof(Char) - number_of_digits; i++) {
          *(out++) = '0';
        }
        out += to_signed(number_of_digits);
        write_number(abs, 16, true, out);
        break;
      }
      }
    }
  }
  return out;
}

template <typename Char>
constexpr size_t count_size_when_escaped(std::basic_string_view<Char> sv) {
  size_t count = 0;
  for (Char c : sv) {
    if (!needs_escape(static_cast<uint32_t>(c))) {
      count += 1;
    } else if (c == '\n' || c == '\r' || c == '\t' || c == '\\' || c == '\'' || c == '"') {
      count += 2;
    } else {
      count += 2 + 2 * sizeof(Char);  // \xAB...
    }
  }
  return count;
}

}  // namespace emio::detail

namespace emio {

/**
 * This class operates on a buffer and allows writing sequences of characters or other kinds of data into it.
 * @tparam Char The character type.
 */
template <typename Char = char>
class writer {
 public:
  /**
   * Constructs a writer with a given buffer.
   * @param buf The buffer.
   */
  constexpr writer(buffer<Char>& buf) noexcept : buf_{buf} {}

  /**
   * Returns the buffer.
   * @return The buffer.
   */
  [[nodiscard]] constexpr buffer<Char>& get_buffer() noexcept {
    return buf_;
  }

  /**
   * Writes a character into the buffer.
   * @param c The character.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char(const Char c) noexcept {
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
  constexpr result<void> write_char_n(const Char c, const size_t n) noexcept {
    // Perform write in multiple chunks, to support buffers with an internal cache.
    size_t remaining_size = n;
    while (remaining_size != 0) {
      EMIO_TRY(const auto area, buf_.get_write_area_of_max(remaining_size));
      std::fill_n(area.data(), area.size(), c);
      remaining_size -= area.size();
    }
    return success;
  }

  /**
   * Writes a character escaped into the buffer.
   * @param c The character.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char_escaped(const Char c) noexcept {
    const std::basic_string_view<Char> sv(&c, 1);
    const size_t required_size = detail::count_size_when_escaped(sv) + 2;
    EMIO_TRY(const auto area, buf_.get_write_area_of(required_size));
    auto it = area.begin();
    *(it++) = '\'';
    it = detail::write_escaped(sv, it);
    *it = '\'';
    return success;
  }

  /**
   * Writes a char sequence into the buffer.
   * @param sv The char sequence.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_str(const std::basic_string_view<Char> sv) noexcept {
    // Perform write in multiple chunks, to support buffers with an internal cache.
    const Char* ptr = sv.data();
    size_t remaining_size = sv.size();
    while (remaining_size != 0) {
      EMIO_TRY(const auto area, buf_.get_write_area_of_max(remaining_size));
      std::copy_n(ptr, area.size(), area.data());
      remaining_size -= area.size();
    }
    return success;
  }

  /**
   * Writes a char sequence escaped into the buffer.
   * @param sv The char sequence.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_str_escaped(const std::basic_string_view<Char> sv) noexcept {
    const size_t required_size = detail::count_size_when_escaped(sv) + 2;
    // TODO: Split writes into multiple chunks.
    //  Not that easy because the remaining size of the sv is != the required output size.
    EMIO_TRY(const auto area, buf_.get_write_area_of(required_size));
    auto it = area.begin();
    *(it++) = '"';
    it = detail::write_escaped(sv, it);
    *(it) = '"';
    return success;
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
  constexpr result<void> write_int(const T integer, const write_int_options& options = {}) noexcept {
    // Reduce code generation by upcasting the integer.
    return write_int_impl(detail::integer_upcast(integer), options);
  }

 private:
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

  buffer<Char>& buf_;
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
 * The reader interprets the char sequence as finite input stream. After every successful operation the read pointer
 * moves on until the last char of the sequence has been consumed.
 * @tparam Char The character type.
 */
template <typename Char>
class reader {
 public:
  /// The view type.
  using view_t = std::basic_string_view<Char>;
  /// The size type.
  using size_type = typename std::basic_string_view<Char>::size_type;
  /// Special value to indicate the end of the view or an error by functions returning an index.
  static constexpr size_type npos = std::basic_string_view<Char>::npos;

  /**
   * Constructs an empty reader.
   */
  constexpr reader() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr reader(std::basic_string<Char>&&) = delete;
  constexpr reader(std::nullptr_t) = delete;
  constexpr reader(int) = delete;

  /**
   * Constructs the reader from any suitable char sequence.
   * @param input The char sequence.
   */
  template <typename Arg>
    requires(std::is_constructible_v<view_t, Arg>)
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload): Is guarded by require clause.
  constexpr explicit(!std::is_convertible_v<Arg, view_t>) reader(Arg&& input) noexcept
      : input_{std::forward<Arg>(input)} {}

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
  [[nodiscard]] constexpr std::basic_string_view<Char> view_remaining() const noexcept {
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
  [[nodiscard]] constexpr view_t read_remaining() noexcept {
    const view_t remaining_view = view_remaining();
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
  constexpr result<Char> peek() noexcept {
    const view_t remaining = view_remaining();
    if (!remaining.empty()) {
      return remaining[0];
    }
    return err::eof;
  }

  /**
   * Reads one char from the stream.
   * @return EOF if the end of the stream has been reached.
   */
  constexpr result<Char> read_char() noexcept {
    const view_t remaining = view_remaining();
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
  constexpr result<view_t> read_n_chars(const size_t n) noexcept {
    const view_t remaining = view_remaining();
    if (remaining.size() >= n) {
      pop(n);
      return view_t{remaining.begin(), remaining.begin() + n};
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
  constexpr result<view_t> read_until_char(const Char delimiter, const read_until_options& options = {}) noexcept {
    return read_until_pos(view_remaining().find(delimiter), options);
  }

  /**
   * Reads multiple chars from the stream until a given char sequence as delimiter is reached or EOF (configurable).
   * @param delimiter The char sequence.
   * @param options The read until options.
   * @return invalid_data if the delimiter hasn't been found and ignore_eof is set to true or EOF if the stream is
   * empty.
   */
  constexpr result<view_t> read_until_str(const std::basic_string_view<Char> delimiter,
                                          const read_until_options& options = {}) {
    return read_until_pos(view_remaining().find(delimiter), options, delimiter.size());
  }

  /**
   * Reads multiple chars from the stream until a char of a given group is reached or EOF (configurable).
   * @param group The char group.
   * @param options The read until options.
   * @return invalid_data if no char has been found and ignore_eof is set to True or EOF if the stream is empty.
   */
  constexpr result<view_t> read_until_any_of(const std::basic_string_view<Char> group,
                                             const read_until_options& options = {}) noexcept {
    return read_until_pos(view_remaining().find_first_of(group), options);
  }

  /**
   * Reads multiple chars from the stream until no char of a given group is reached or EOF (configurable).
   * @param group The char group.
   * @param options The read until options.
   * @return invalid_data if a char not in the group has been found and ignore_eof is set to True or EOF if the stream
   * is empty.
   */
  constexpr result<view_t> read_until_none_of(const std::basic_string_view<Char> group,
                                              const read_until_options& options = {}) noexcept {
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
    requires(std::is_invocable_r_v<bool, Predicate, Char>)
  constexpr result<view_t> read_until(Predicate&& predicate, const read_until_options& options = {}) noexcept(
      std::is_nothrow_invocable_r_v<bool, Predicate, Char>) {
    const view_t sv = view_remaining();
    const auto begin = sv.data();
    const auto end = sv.data() + sv.size();
    const auto it = std::find_if(begin, end, predicate);
    const auto pos = (it != end) ? std::distance(begin, it) : 0;
    return read_until_pos(static_cast<size_t>(pos), options);
  }

  /**
   * Reads one char from the stream if the char matches the expected one.
   * @param c The expected char.
   * @return invalid_data if the chars don't match or EOF if the end of the stream has been reached.
   */
  constexpr result<char> read_if_match_char(const char c) noexcept {
    EMIO_TRY(const Char p, peek());
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
  constexpr result<view_t> read_if_match_str(const std::basic_string_view<Char> sv) noexcept {
    const view_t remaining = view_remaining();
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
  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<T> parse_int_impl(const int base) {
    if (!detail::is_valid_number_base(base)) {
      return err::invalid_argument;
    }

    T value{};
    T maybe_overflowed_value{};
    T signed_flag{1};

    EMIO_TRY(Char c, peek());
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

  constexpr result<view_t> read_until_pos(size_t pos, const read_until_options& options,
                                          const size_type delimiter_size = 1) noexcept {
    const view_t remaining = view_remaining();
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
  std::basic_string_view<Char> input_;
};

// Deduction guides.
template <typename Char>
reader(std::basic_string_view<Char>) -> reader<Char>;

template <typename Char, typename Traits, typename Alloc>
reader(std::basic_string<Char, Traits, Alloc>) -> reader<Char>;

template <typename Char>
reader(const Char*) -> reader<Char>;

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
  int width{0};
  int precision{no_precision};
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

template <typename Char, input_validation>
class parser_base {
 public:
  explicit constexpr parser_base(reader<Char>& format_rdr) noexcept : format_rdr_{format_rdr} {}

  parser_base(const parser_base&) = delete;
  parser_base(parser_base&&) = delete;
  parser_base& operator=(const parser_base&) = delete;
  parser_base& operator=(parser_base&&) = delete;

  constexpr virtual ~parser_base() = default;

  constexpr result<void> parse(uint8_t& arg_nbr) noexcept {
    while (true) {
      result<Char> res = format_rdr_.read_char();
      if (res == err::eof) {
        return success;
      }
      if (!res) {
        return res;
      }
      Char c = res.assume_value();
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
  virtual constexpr result<void> process(Char c) noexcept = 0;

  reader<Char>& format_rdr_;

 private:
  constexpr result<void> parse_replacement_field(uint8_t& arg_nbr) noexcept {
    EMIO_TRYV(parse_field_name(arg_nbr));

    EMIO_TRY(Char c, format_rdr_.peek());
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
    EMIO_TRY(Char c, format_rdr_.peek());
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

template <typename Char>
class parser_base<Char, input_validation::disabled> {
 public:
  explicit constexpr parser_base(reader<Char>& format_rdr) noexcept : format_rdr_{format_rdr} {}

  parser_base(const parser_base& other) = delete;
  parser_base(parser_base&& other) = delete;
  parser_base& operator=(const parser_base& other) = delete;
  parser_base& operator=(parser_base&& other) = delete;

  constexpr virtual ~parser_base() = default;

  constexpr result<void> parse(uint8_t& arg_nbr) noexcept {
    while (true) {
      result<Char> res = format_rdr_.read_char();
      if (res == err::eof) {
        return success;
      }
      Char c = res.assume_value();
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
  virtual constexpr result<void> process(Char c) noexcept = 0;

  reader<Char>& format_rdr_;

 private:
  constexpr result<void> parse_replacement_field(uint8_t& arg_nbr) noexcept {
    parse_field_name(arg_nbr);
    Char c = format_rdr_.peek().assume_value();
    if (c == '}') {
      return success;
    }
    format_rdr_.pop();
    return success;
  }

  constexpr void parse_field_name(uint8_t& arg_nbr) noexcept {
    Char c = format_rdr_.peek().assume_value();
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

namespace emio {

template <typename>
class formatter;

namespace detail::format {

//
// Write args.
//

inline constexpr result<void> write_padding_left(writer<char>& wtr, format_specs& specs, size_t width) {
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

inline constexpr result<void> write_padding_right(writer<char>& wtr, format_specs& specs) {
  if (specs.width == 0 || (specs.align != alignment::left && specs.align != alignment::center)) {
    return success;
  }
  return wtr.write_char_n(specs.fill, static_cast<size_t>(specs.width));
}

template <alignment DefaultAlign, typename Func>
constexpr result<void> write_padded(writer<char>& wtr, format_specs& specs, size_t width, Func&& func) {
  if (specs.align == alignment::none) {
    specs.align = DefaultAlign;
  }
  EMIO_TRYV(write_padding_left(wtr, specs, width));
  EMIO_TRYV(func());
  return write_padding_right(wtr, specs);
}

inline constexpr result<std::pair<std::string_view, writer<char>::write_int_options>> make_write_int_options(
    char spec_type) noexcept {
  using namespace alternate_form;

  std::string_view prefix;
  writer<char>::write_int_options options{};

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

inline constexpr result<void> write_sign_and_prefix(writer<char>& wtr, const format_specs& specs, bool negative,
                                                    std::string_view prefix) {
  if (negative) {
    EMIO_TRYV(wtr.write_char('-'));
  } else if (specs.sign == '+' || specs.sign == ' ') {
    EMIO_TRYV(wtr.write_char(specs.sign));
  }
  if (specs.alternate_form && !prefix.empty()) {
    EMIO_TRYV(wtr.write_str(prefix));
  }
  return success;
}

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const Arg& arg) noexcept {
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
  const bool negative = detail::is_negative(arg);
  const size_t number_of_digits = detail::get_number_of_digits(abs_number, options.base);

  size_t total_length = number_of_digits;
  if (specs.alternate_form) {
    total_length += prefix.size();
  }
  if (negative || specs.sign == ' ' || specs.sign == '+') {
    total_length += 1;
  }

  if (specs.zero_flag) {
    EMIO_TRYV(write_sign_and_prefix(wtr, specs, negative, prefix));
  }

  return write_padded<alignment::right>(
      wtr, specs, total_length, [&, &prefix = prefix, &options = options]() -> result<void> {
        if (!specs.zero_flag) {
          EMIO_TRYV(write_sign_and_prefix(wtr, specs, negative, prefix));
        }

        auto& buf = wtr.get_buffer();
        EMIO_TRY(auto area, buf.get_write_area_of(number_of_digits));
        write_number(abs_number, options.base, options.upper_case, area.begin() + detail::to_signed(number_of_digits));
        return success;
      });
}

// template <typename Arg>
// requires(std::is_floating_point_v<Arg>)
//      result<void> write_arg(writer<char>& wtr, const format_specs&
//     specs,
//                                          const Arg& arg) noexcept {
//	result<void> err;
//	std::chars_format fmt;
//	switch (specs.type) {
//	case no_type:
//	case 'f':
//		fmt = std::chars_format::fixed;
//		break;
//		//	case 'G': upper
//	case 'g':
//		fmt = std::chars_format::general;
//		break;
//		//	case 'E': upper
//	case 'e':
//		fmt = std::chars_format::scientific;
//		break;
//		//	case 'F': upper
//		// case 'A': upper
//	case 'a':
//		fmt = std::chars_format::hex;
//		err = wtr.write("0x");
//		break;
//	default:
//		err = err::invalid_format;
//	}
//	if (!err.has_error()) {
//		if (specs.precision == NoPrecision) {
//			if (specs.type == no_type) {
//				err = wtr.write(arg);
//			} else {
//				err = wtr.write(arg, fmt);
//			}
//		} else {
//			err = wtr.write(arg, fmt, specs.precision);
//		}
//	}
//	return err;
// return err::invalid_format;
//}

inline constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const std::string_view arg) noexcept {
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
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const Arg arg) noexcept {
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
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, Arg arg) noexcept {
  specs.alternate_form = true;
  specs.type = 'x';
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): valid cast
  return write_arg(wtr, specs, reinterpret_cast<uintptr_t>(arg));
}

template <typename Arg>
  requires(std::is_same_v<Arg, bool>)
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, Arg arg) noexcept {
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
inline constexpr result<void> validate_format_specs(reader<char>& rdr, format_specs& specs) noexcept {
  EMIO_TRY(char c, rdr.read_char());
  if (c == '}') {  // Format end.
    return success;
  }
  if (c == '{') {  // No dynamic spec support.
    return err::invalid_format;
  }

  bool width_required = false;
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
      width_required = true;
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
      width_required = true;
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
  if (c == '0') {          // Zero flag.
    if (width_required) {  // Fill and zero flag doesn't make sense.
      return err::invalid_format;
    }
    specs.fill = '0';
    specs.align = alignment::right;
    specs.zero_flag = true;
    width_required = true;
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    EMIO_TRY(specs.width, rdr.parse_int<int>());
    EMIO_TRY(c, rdr.read_char());
  } else if (width_required) {  // Width was required.
    return err::invalid_format;
  }
  if (c == '.') {  // Precision.
    EMIO_TRY(specs.precision, rdr.parse_int<int>());
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

inline constexpr result<void> parse_format_specs(reader<char>& rdr, format_specs& specs) noexcept {
  char c = rdr.read_char().assume_value();
  if (c == '}') {  // Format end.
    return success;
  }

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
  if (c == '0') {  // Zero flag.
    specs.fill = '0';
    specs.align = alignment::right;
    specs.zero_flag = true;
    c = rdr.read_char().assume_value();
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    specs.width = rdr.parse_int<int>().assume_value();
    c = rdr.read_char().assume_value();
  }
  if (c == '.') {  // Precision.
    specs.precision = rdr.parse_int<int>().assume_value();
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
  switch (specs.type) {
  case no_type:
  case 'f':
  case 'e':
  case 'g':
  case 'a':
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
                                    {
                                      formatter<T>::validate(std::declval<reader<char>&>())
                                      } -> std::same_as<result<void>>;
                                  };

template <typename T>
concept has_any_validate_function_v =
    requires { &formatter<T>::validate; } || std::is_member_function_pointer_v<decltype(&formatter<T>::validate)> ||
    requires { std::declval<formatter<T>>().validate(std::declval<reader<char>&>()); };

template <typename Arg>
constexpr result<void> validate_for(reader<char>& format_is) noexcept {
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
    std::is_same_v<T, void*> || std::is_constructible_v<std::string_view, T>;

template <input_validation FormatStringValidation, typename T>
concept formatter_parse_supports_format_string_validation =
    requires(T formatter) { formatter.template parse<FormatStringValidation>(std::declval<reader<char>>()); };

template <input_validation FormatStringValidation, typename T>
inline constexpr result<void> invoke_formatter_parse(T& formatter, reader<char>& format_is) noexcept {
  if constexpr (formatter_parse_supports_format_string_validation<FormatStringValidation, T>) {
    return formatter.template parse<FormatStringValidation>(format_is);
  } else {
    return formatter.parse(format_is);
  }
}

}  // namespace detail::format
}  // namespace emio

namespace emio {

/**
 * Class template that defines formatting rules for a given type.
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
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param rdr The format reader.
   * @return Success if the format spec is valid and could be parsed.
   */
  constexpr result<void> parse(reader<char>& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to format the object of this type according to the parsed format specs.
   * @param wtr The output writer.
   * @param arg The argument to format.
   * @return Success if the formatting could be done.
   */
  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return wtr.write_int(sizeof(arg));
  }
};

/**
 * Formatter for most common unambiguity types.
 * This includes:
 * - boolean
 * - void* / nullptr
 * - integral types
 * - floating-point types (TODO)
 * - chrono duration (TODO)
 * - ranges of the above types (TODO)
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_core_type_v<T>)
class formatter<T> {
 public:
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
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

  constexpr result<void> parse(reader<char>& rdr) noexcept {
    return detail::format::parse_format_specs(rdr, specs_);
  }

  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return write_arg(wtr, specs_, arg);
  }

 private:
  detail::format::format_specs specs_{};
};

/**
 * Formatter for unscoped enum types to there underlying type.
 * @tparam T The unscoped enum type.
 */
template <typename T>
  requires(std::is_enum_v<T> && std::is_convertible_v<T, std::underlying_type_t<T>>)
class formatter<T> : public formatter<std::underlying_type_t<T>> {
 public:
  result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return formatter<std::underlying_type_t<T>>::format(wtr, static_cast<std::underlying_type_t<T>>(arg));
  }
};

}  // namespace emio

namespace emio::detail::format {

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

  result<void> validate(reader<char>& format_is) const noexcept {
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

    virtual result<void> validate(reader<char>& format_is) const noexcept = 0;

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

    result<void> validate(reader<char>& format_is) const noexcept override {
      return validate_for<std::decay_t<T>>(format_is);
    }

   protected:
    ~model_t() = default;
  };

  std::aligned_storage_t<sizeof(model_t<int>)> storage_;
};

/**
 * Format arguments just for format string validation.
 */
template <typename Char>
class format_validation_args {
 public:
  format_validation_args(const format_validation_args&) = delete;
  format_validation_args(format_validation_args&&) = delete;
  format_validation_args& operator=(const format_validation_args&) = delete;
  format_validation_args& operator=(format_validation_args&&) = delete;
  ~format_validation_args() = default;

  [[nodiscard]] std::basic_string_view<Char> get_format_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const format_validation_arg> get_args() const noexcept {
    return args_;
  }

 protected:
  format_validation_args(std::basic_string_view<Char> format_str, std::span<const format_validation_arg> args)
      : format_str_{format_str}, args_{args} {}

 private:
  std::basic_string_view<Char> format_str_;
  std::span<const detail::format::format_validation_arg> args_;
};

/**
 * Format arguments storage just for format string validation.
 */
template <typename Char, size_t NbrOfArgs>
class basic_format_validation_args_storage : public format_validation_args<Char> {
 public:
  template <typename... Args>
  basic_format_validation_args_storage(std::basic_string_view<Char> str, const Args&... args)
      : format_validation_args<Char>{str, args_storage_}, args_storage_{format_validation_arg{args}...} {}

  basic_format_validation_args_storage(const basic_format_validation_args_storage&) = delete;
  basic_format_validation_args_storage(basic_format_validation_args_storage&&) = delete;
  basic_format_validation_args_storage& operator=(const basic_format_validation_args_storage&) = delete;
  basic_format_validation_args_storage& operator=(basic_format_validation_args_storage&&) = delete;
  ~basic_format_validation_args_storage() = default;

 private:
  std::array<format_validation_arg, NbrOfArgs> args_storage_;
};

template <typename Char, typename... Args>
basic_format_validation_args_storage<Char, sizeof...(Args)> make_format_validation_args(
    std::basic_string_view<Char> format_str) {
  return {format_str, std::type_identity<Args>{}...};
}

/**
 * Type erased format argument for formatting.
 */
template <typename Char>
class basic_format_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit basic_format_arg(const T& value) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<unified_type_t<T>>*>(&storage_), value);
  }

  basic_format_arg(const basic_format_arg&) = delete;
  basic_format_arg(basic_format_arg&&) = delete;
  basic_format_arg& operator=(const basic_format_arg&) = delete;
  basic_format_arg& operator=(basic_format_arg&&) = delete;
  ~basic_format_arg() = default;  // No destructor & delete call to concept_t because model_t holds only a reference.

  result<void> format(writer<Char>& wtr, reader<Char>& format_is) const noexcept {
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

    virtual result<void> format(writer<Char>& wtr, reader<Char>& format_is) const noexcept = 0;

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

    result<void> format(writer<Char>& wtr, reader<Char>& format_is) const noexcept override {
      formatter<std::decay_t<T>> formatter;
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
template <typename Char>
class basic_format_args {
 public:
  basic_format_args(const basic_format_args&) = delete;
  basic_format_args(basic_format_args&&) = delete;
  basic_format_args& operator=(const basic_format_args&) = delete;
  basic_format_args& operator=(basic_format_args&&) = delete;
  ~basic_format_args() = default;

  result<std::basic_string_view<Char>> get_format_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const basic_format_arg<Char>> get_args() const noexcept {
    return args_;
  }

 protected:
  basic_format_args(result<std::basic_string_view<Char>> format_str, std::span<const basic_format_arg<Char>> args)
      : format_str_{format_str}, args_{args} {}

 private:
  result<std::basic_string_view<Char>> format_str_;
  std::span<const basic_format_arg<Char>> args_;
};

/**
 * Format arguments storage for formatting.
 */
template <typename Char, size_t NbrOfArgs>
class basic_format_args_storage : public basic_format_args<Char> {
 public:
  template <typename... Args>
  basic_format_args_storage(result<std::basic_string_view<Char>> str, const Args&... args)
      : basic_format_args<Char>{str, args_storage_}, args_storage_{basic_format_arg<Char>{args}...} {}

  basic_format_args_storage(const basic_format_args_storage&) = delete;
  basic_format_args_storage(basic_format_args_storage&&) = delete;
  basic_format_args_storage& operator=(const basic_format_args_storage&) = delete;
  basic_format_args_storage& operator=(basic_format_args_storage&&) = delete;
  ~basic_format_args_storage() = default;

 private:
  std::array<basic_format_arg<Char>, NbrOfArgs> args_storage_;
};

}  // namespace emio::detail::format

namespace emio::detail::format {

template <typename Char>
class format_parser final : public parser_base<Char, input_validation::disabled> {
 public:
  explicit constexpr format_parser(writer<Char>& wtr, reader<Char>& format_rdr) noexcept
      : parser_base<Char, input_validation::disabled>{format_rdr}, wtr_{wtr} {}

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

  writer<Char>& wtr_;
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
template <typename Char>
constexpr format_parser<Char>::~format_parser() noexcept = default;

template <typename Char>
class format_specs_checker final : public parser_base<Char, input_validation::enabled> {
 public:
  using parser_base<Char, input_validation::enabled>::parser_base;

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
    return validate_for<Arg>(this->format_rdr_);
  }
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
template <typename Char>
constexpr format_specs_checker<Char>::~format_specs_checker() noexcept = default;

template <typename Char>
[[nodiscard]] bool validate_format_string_fallback(const format_validation_args<Char>& args) noexcept {
  reader<Char> format_rdr{args.get_format_str()};
  format_specs_checker<Char> fh{format_rdr};
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

template <typename... Args, typename Char>
[[nodiscard]] constexpr bool validate_format_string(std::basic_string_view<Char> format_str) noexcept {
  if (Y_EMIO_IS_CONST_EVAL) {
    reader<Char> format_rdr{format_str};
    format_specs_checker<Char> fh{format_rdr};
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
    return validate_format_string_fallback(make_format_validation_args<Char, Args...>(format_str));
  }
}

}  // namespace emio::detail::format

namespace emio {

/**
 * This class represents a not yet validated format string, which has to be validated at runtime.
 * @tparam Char The character type.
 */
template <typename Char>
class runtime {
 public:
  /**
   * Constructs an empty runtime format string.
   */
  constexpr runtime() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr runtime(std::basic_string<Char>&&) = delete;
  constexpr runtime(std::nullptr_t) = delete;
  constexpr runtime(int) = delete;

  /**
   * Constructs the runtime format string from any suitable char sequence.
   * @param str The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::basic_string_view<Char>, S>)
  explicit constexpr runtime(const S& str) : str_{str} {}

  /**
   * Obtains a view over the runtime format string.
   * @return The view.
   */
  [[nodiscard]] constexpr std::basic_string_view<Char> view() const noexcept {
    return str_;
  }

 private:
  std::basic_string_view<Char> str_;
};

// Deduction guides.
template <typename Char>
runtime(std::basic_string_view<Char>) -> runtime<Char>;

template <typename Char, typename Traits, typename Alloc>
runtime(std::basic_string<Char, Traits, Alloc>) -> runtime<Char>;

template <typename Char>
runtime(const Char*) -> runtime<Char>;

template <typename Char, typename... Args>
class basic_valid_format_string;

/**
 * This class represents a validated format string. The validation happens at object construction.
 * @tparam Char The character type.
 * @tparam Args The argument types to format.
 */
template <typename Char, typename... Args>
class basic_format_string {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::basic_string_view<Char>, S>)
  consteval basic_format_string(const S& s) {
    std::basic_string_view<Char> str{s};
    if (detail::format::validate_format_string<Args...>(str)) {
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
  constexpr basic_format_string(runtime<Char> s) {
    std::basic_string_view<Char> str{s.view()};
    if (detail::format::validate_format_string<Args...>(str)) {
      str_ = str;
    }
  }

  /**
   * Returns the validated format string as view.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::basic_string_view<Char>> get() const noexcept {
    return str_;
  }

  /**
   * Returns format string as valid one.
   * @return The valid format string or invalid_format if the validation failed.
   */
  constexpr result<basic_valid_format_string<Char, Args...>> as_valid() const noexcept {
    if (str_.has_value()) {
      return basic_valid_format_string<Char, Args...>{valid, str_.assume_value()};
    }
    return err::invalid_format;
  }

 protected:
  static constexpr struct valid_t {
  } valid{};

  constexpr explicit basic_format_string(valid_t /*unused*/, std::basic_string_view<Char> s) : str_{s} {}

 private:
  result<std::basic_string_view<Char>> str_{err::invalid_format};  ///< Validated format string.
};

/**
 * This class represents a valid format string. The validation happens at object construction.
 * @tparam Char The character type.
 * @tparam Args The argument types to format.
 */
template <typename Char, typename... Args>
class basic_valid_format_string : public basic_format_string<Char, Args...> {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::basic_string_view<Char>, S>)
  consteval basic_valid_format_string(const S& s) : basic_format_string<Char, Args...>{s} {}

  /**
   * Constructs and validates a format string at runtime.
   * @param s The format string.
   * @return The valid format string or invalid_format if the validation failed.
   */
  template <typename S>
    requires(std::is_constructible_v<std::basic_string_view<Char>, S>)
  static constexpr result<basic_valid_format_string<Char, Args...>> from(const S& s) {
    std::basic_string_view<Char> str{s};
    if (!detail::format::validate_format_string<Args...>(str)) {
      return err::invalid_format;
    }
    return basic_valid_format_string{valid, str};
  }

 private:
  friend class basic_format_string<Char, Args...>;

  using valid_t = typename basic_format_string<Char, Args...>::valid_t;
  using basic_format_string<Char, Args...>::valid;

  explicit constexpr basic_valid_format_string(valid_t /*unused*/, std::basic_string_view<Char> s)
      : basic_format_string<Char, Args...>{valid, s} {}
};

// Alias template types.
template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = basic_valid_format_string<char, std::type_identity_t<Args>...>;

}  // namespace emio

namespace emio::detail::format {

// Non constexpr version.
template <typename Char>
result<void> vformat_to(buffer<Char>& buf, const basic_format_args<Char>& args) noexcept {
  EMIO_TRY(const std::basic_string_view<Char> str, args.get_format_str());
  reader<Char> format_rdr{str};
  writer<char> wtr{buf};
  format_parser<Char> fh{wtr, format_rdr};
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
template <typename Char, typename... Args>
constexpr result<void> format_to(buffer<Char>& buf, basic_format_string<Char, Args...> format_str,
                                 const Args&... args) noexcept {
  EMIO_TRY(std::basic_string_view<Char> str, format_str.get());
  reader<Char> format_rdr{str};
  writer<Char> wtr{buf};
  format_parser<Char> fh{wtr, format_rdr};
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
template <typename Char>
using basic_format_args = detail::format::basic_format_args<Char>;

// Alias type.
using format_args = detail::format::basic_format_args<char>;

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
[[nodiscard]] detail::format::basic_format_args_storage<char, sizeof...(Args)> make_format_args(
    format_string<Args...> format_str, const Args&... args) noexcept {
  return {format_str.get(), args...};
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param args The format args with the format string.
 * @return The total number of characters in the formatted string or invalid_format if the format string validation
 * failed.
 */
inline result<size_t> vformatted_size(format_args&& args) noexcept {
  detail::basic_counting_buffer<char> buf{};
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
  detail::basic_counting_buffer<char> buf{};
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
  requires(std::is_same_v<T, runtime<char>> || std::is_same_v<T, format_string<Args...>>)
constexpr result<size_t> formatted_size(T format_str, const Args&... args) noexcept {
  detail::basic_counting_buffer<char> buf{};
  basic_format_string<char, Args...> str{format_str};
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
  requires(std::is_base_of_v<buffer<char>, Buffer>)
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
inline result<void> vformat_to(writer<char>& wrt, const format_args& args) noexcept {
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
  requires(std::is_base_of_v<buffer<char>, Buffer>)
constexpr result<void> format_to(Buffer& buf, format_string<Args...> format_str, const Args&... args) noexcept {
  if (Y_EMIO_IS_CONST_EVAL) {
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
constexpr result<void> format_to(writer<char>& wrt, format_string<Args...> format_str, const Args&... args) noexcept {
  if (Y_EMIO_IS_CONST_EVAL) {
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
  if (Y_EMIO_IS_CONST_EVAL) {
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
  string_buffer<char> buf;
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
  return vformat(make_format_args(format_str, args...)).value();
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The string on success or invalid_format if the format string validation
 * failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime<char>> || std::is_same_v<T, format_string<Args...>>)
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
  if (Y_EMIO_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  tout = buf.out();
  return format_to_n_result<OutputIt>{tout.out(), tout.count()};
}

}  // namespace emio

#endif  // EMIO_Z_MAIN_H
