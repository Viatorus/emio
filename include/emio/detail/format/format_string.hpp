//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>
#include <type_traits>

#include "parser.hpp"

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
