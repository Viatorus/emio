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
 * This class represents a validated format string. The format string is either valid or not.
 * @note The validation happens at object construction.
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
  consteval basic_format_string(const S& s) noexcept {
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
  constexpr basic_format_string(runtime<Char> s) noexcept {
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

  constexpr explicit basic_format_string(valid_t /*unused*/, std::basic_string_view<Char> s) noexcept : str_{s} {}

 private:
  result<std::basic_string_view<Char>> str_{err::invalid_format};  ///< Validated format string.
};

/**
 * This class represents a validated format string. The format string can only be valid.
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
  consteval basic_valid_format_string(const S& s) noexcept : basic_format_string<Char, Args...>{s} {}

  /**
   * Constructs and validates a format string at runtime.
   * @param s The format string.
   * @return The valid format string or invalid_format if the validation failed.
   */
  template <typename S>
    requires(std::is_constructible_v<std::basic_string_view<Char>, S>)
  static constexpr result<basic_valid_format_string<Char, Args...>> from(const S& s) noexcept {
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

  explicit constexpr basic_valid_format_string(valid_t /*unused*/, std::basic_string_view<Char> s) noexcept
      : basic_format_string<Char, Args...>{valid, s} {}
};

// Alias template types.
template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = basic_valid_format_string<char, std::type_identity_t<Args>...>;

}  // namespace emio
