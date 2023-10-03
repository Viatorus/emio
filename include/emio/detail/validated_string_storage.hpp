//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstring>
#include <string_view>

#include "../result.hpp"

namespace emio::detail {

inline constexpr bool check_if_plain_string(const std::string_view& s) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    return s.find_first_of("{}"sv) == npos;
  } else {
    return std::memchr(s.data(), static_cast<int>('{'), s.size()) == nullptr &&
           std::memchr(s.data(), static_cast<int>('}'), s.size()) == nullptr;
  }
}

class validated_string_storage {
 public:
  template <typename Trait, typename... Args>
  static constexpr validated_string_storage from(const std::string_view& s) noexcept {
    if constexpr (sizeof...(Args) == 0) {
      if (check_if_plain_string(s)) {
        return {{true, s}};
      }
    }
    if (Trait::template validate_string<Args...>(s)) {
      return {{false, s}};
    }
    return {};
  }

  constexpr validated_string_storage() noexcept = default;

  /**
   * Returns if it is just a plain string.
   * @return True, if the string does not contain any escape sequences or replacement fields, otherwise false.
   */
  [[nodiscard]] constexpr bool is_plain_str() const noexcept {
    return str_.first;
  }

  /**
   * Returns the validated format/scan string.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::string_view> get() const noexcept {
    return str_.second;
  }

 private:
  // NOLINTNEXTLINE(modernize-pass-by-value): false-positive since no dynamic allocation takes place
  constexpr validated_string_storage(const std::pair<bool, result<std::string_view>>& str) noexcept : str_{str} {}

  // Wonder why pair and not two variables? Look at this bug report: https://github.com/llvm/llvm-project/issues/67731
  std::pair<bool, result<std::string_view>> str_{false, err::invalid_format};
};

}  // namespace emio::detail
