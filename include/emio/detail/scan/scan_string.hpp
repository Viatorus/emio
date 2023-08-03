//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>
#include <type_traits>

#include "parser.hpp"

#include "../format/format_string.hpp" // TODO: refactor

namespace emio::detail::scan {

/**
 * This class represents a validated scan string. The format string is either valid or not.
 * @note The validation happens at object construction.
 * @tparam Args The argument types to format.
 */
template <typename... Args>
class scan_string {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval scan_string(const S& s) noexcept {
    std::string_view str{s};
    if (validate_scan_string<Args...>(str)) {
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
  constexpr scan_string(detail::format::runtime_format_string s) noexcept {
    std::string_view str{s.view()};
    if (validate_scan_string<Args...>(str)) {
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

 private:
  result<std::string_view> str_{err::invalid_format};
};

}  // namespace emio::detail::scan