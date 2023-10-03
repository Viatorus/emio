//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
#include "../../writer.hpp"
#include "../validated_string.hpp"
#include "args.hpp"
#include "parser.hpp"

namespace emio::detail::format {

struct format_trait {
  template <typename... Args>
  [[nodiscard]] static constexpr bool validate_string(std::string_view format_str) noexcept {
    if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
      return validate<format_specs_checker>(format_str, sizeof...(Args), std::type_identity<Args>{}...);
    } else {
      return validate<format_specs_checker>(format_str, sizeof...(Args),
                                            make_validation_args<format_validation_arg, Args...>());
    }
  }
};

template <typename... Args>
using format_string = validated_string<format_trait, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = valid_string<format_trait, std::type_identity_t<Args>...>;

// Non constexpr version.
inline result<void> vformat_to(buffer& buf, const format_args& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_str());
  writer wtr{buf};
  if (args.is_plain_str()) {
    return wtr.write_str(str);
  }
  return parse<format_parser>(str, wtr, args);
}

// Constexpr version.
template <typename... Args>
constexpr result<void> format_to(buffer& buf, format_string<Args...> format_string, const Args&... args) noexcept {
  EMIO_TRY(const std::string_view str, format_string.get());
  writer wtr{buf};
  if (format_string.is_plain_str()) {
    return wtr.write_str(str);
  }
  return parse<format_parser>(str, wtr, args...);
}

}  // namespace emio::detail::format
