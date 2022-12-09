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
   * Returns the validated format string.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::basic_string_view<Char>> get() const noexcept {
    return str_;
  }

 private:
  result<std::basic_string_view<Char>> str_{err::invalid_format};  ///< Validated format string.
};

// Alias template types.
template <typename... Args>
using format_string = basic_format_string<char, std::type_identity_t<Args>...>;

}  // namespace emio
