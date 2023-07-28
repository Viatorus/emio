//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>
#include <type_traits>

#include "detail/format/parser.hpp"

namespace emio {

/**
 * This class represents a not yet validated format string, which has to be validated at runtime.
 */
class runtime {
 public:
  /**
   * Constructs an empty runtime format string.
   */
  constexpr runtime() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr runtime(std::string&&) = delete;
  constexpr runtime(std::nullptr_t) = delete;
  constexpr runtime(int) = delete;

  /**
   * Constructs the runtime format string from any suitable char sequence.
   * @param str The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  constexpr explicit runtime(const S& str) : str_{str} {}

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
class basic_valid_format_string;

/**
 * This class represents a validated format string. The format string is either valid or not.
 * @note The validation happens at object construction.
 * @tparam Args The argument types to format.
 */
template <typename... Args>
class basic_format_string {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval basic_format_string(const S& s) noexcept {
    std::string_view str{s};
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
  constexpr basic_format_string(runtime s) noexcept {
    std::string_view str{s.view()};
    if (detail::format::validate_format_string<Args...>(str)) {
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
  constexpr result<basic_valid_format_string<Args...>> as_valid() const noexcept {
    if (str_.has_value()) {
      return basic_valid_format_string<Args...>{valid, str_.assume_value()};
    }
    return err::invalid_format;
  }

 protected:
  static constexpr struct valid_t {
  } valid{};

  constexpr explicit basic_format_string(valid_t /*unused*/, std::string_view s) noexcept : str_{s} {}

 private:
  result<std::string_view> str_{err::invalid_format};  ///< Validated format string.
};

/**
 * This class represents a validated format string. The format string can only be valid.
 * @tparam Args The argument types to format.
 */
template <typename... Args>
class basic_valid_format_string : public basic_format_string<Args...> {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval basic_valid_format_string(const S& s) noexcept : basic_format_string<Args...>{s} {}

  /**
   * Constructs and validates a format string at runtime.
   * @param s The format string.
   * @return The valid format string or invalid_format if the validation failed.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  static constexpr result<basic_valid_format_string<Args...>> from(const S& s) noexcept {
    std::string_view str{s};
    if (!detail::format::validate_format_string<Args...>(str)) {
      return err::invalid_format;
    }
    return basic_valid_format_string{valid, str};
  }

 private:
  friend class basic_format_string<Args...>;

  using valid_t = typename basic_format_string<Args...>::valid_t;
  using basic_format_string<Args...>::valid;

  constexpr explicit basic_valid_format_string(valid_t /*unused*/, std::string_view s) noexcept
      : basic_format_string<Args...>{valid, s} {}
};

// Alias template types.
template <typename... Args>
using format_string = basic_format_string<std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = basic_valid_format_string<std::type_identity_t<Args>...>;

}  // namespace emio
