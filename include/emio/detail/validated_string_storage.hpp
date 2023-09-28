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
    validated_string_storage storage{};
    if constexpr (sizeof...(Args) == 0) {
      if (check_if_plain_string(s)) {
        storage.is_plain_str_ = true;
        storage.str_ = s;
      } else if (Trait::template validate_string<Args...>(s)) {
        storage.str_ = s;
      }
    } else {
      if (Trait::template validate_string<Args...>(s)) {
        storage.str_ = s;
      }
    }
    return storage;
  }

  constexpr validated_string_storage() noexcept = default;

  /**
   * Returns the validated format/scan string.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::string_view> get() const noexcept {
    return str_;
  }

  /**
   * Returns if it is just a plain string.
   * @return True, if the string does not contain any escape sequences or replacement fields, otherwise false.
   */
  [[nodiscard]] constexpr bool is_plain_str() const noexcept {
    return is_plain_str_;
  }

 protected:
  static constexpr struct valid_t {
  } valid{};

  constexpr explicit validated_string_storage(valid_t /*unused*/, const validated_string_storage& other) noexcept
      : validated_string_storage{other} {}

 private:
  bool is_plain_str_{false};
  result<std::string_view> str_{err::invalid_format};  ///< Validated string.
};

}  // namespace emio::detail
