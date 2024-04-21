//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <concepts>
#include <cstdint>
#include <optional>
#if __STDC_HOSTED__
#  include <stdexcept>
#endif
#include <type_traits>

#include "detail/predef.hpp"

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
#if __STDC_HOSTED__
class bad_result_access : public std::logic_error {
 public:
  /**
   * Constructs the bad result access from a message.
   * @param msg The exception message.
   */
  explicit bad_result_access(const std::string_view& msg) : logic_error{std::string{msg}} {}
};
#else
class bad_result_access : public std::exception {
 public:
  /**
   * Constructs the bad result access from a message.
   * @param msg The exception message.
   */
  explicit bad_result_access(const std::string_view& msg) : msg_{msg} {}

  [[nodiscard]] const char* what() const noexcept override {
    return msg_.data();
  }

 private:
  std::string_view msg_;
};
#endif

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
constexpr bool operator==(const result<T>& left, const result<U>& right) noexcept {
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
constexpr bool operator==(const result<T>& left, const U& right) noexcept {
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
constexpr bool operator==(const result<T>& left, const err right) noexcept {
  return left.has_error() && left.error() == right;
}

}  // namespace emio
