//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
#include "../../writer.hpp"
#include "../validated_string.hpp"
#include "args.hpp"
#include "parser.hpp"

namespace emio::detail::scan {

struct scan_trait {
  template <typename... Args>
  [[nodiscard]] static constexpr bool validate_string(std::string_view format_str) noexcept {
    if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
      return validate<scan_specs_checker>(format_str, sizeof...(Args), std::type_identity<Args>{}...);
    } else {
      return validate<scan_specs_checker>(format_str, sizeof...(Args),
                                          make_validation_args<scan_validation_arg, Args...>());
    }
  }
};

template <typename... Args>
using format_string = validated_string<scan_trait, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = valid_string<scan_trait, std::type_identity_t<Args>...>;

inline result<void> vscan_from(reader& in, const scan_args& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_str());
  if (args.is_plain_str()) {
    return in.read_if_match_str(str);
  }
  return parse<scan_parser>(str, in, args);
}

template <typename... Args>
constexpr result<void> scan_from(reader& in, format_string<Args...> format_str, Args&... args) noexcept {
  EMIO_TRY(const std::string_view str, format_str.get());
  if (format_str.is_plain_str()) {
    return in.read_if_match_str(str);
  }
  return parse<scan_parser>(str, in, args...);
}

}  // namespace emio::detail::scan
