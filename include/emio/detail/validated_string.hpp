//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>

#include "../result.hpp"

namespace emio {

class validatedstring {
 public:
  constexpr validatedstring() {}

  /**
   * Returns the validated format/scan string as view.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::string_view> get() const noexcept {
    return str_;
  }

  [[nodiscard]] constexpr bool is_plain_str() const noexcept {
    return false;
  }

 protected:
  static constexpr struct valid_t {
  } valid{};

  constexpr explicit validatedstring(valid_t /*unused*/, std::string_view s) noexcept : str_{s} {}

  result<std::string_view> str_{err::invalid_format};  ///< Validated string.
};

}  // namespace emio
