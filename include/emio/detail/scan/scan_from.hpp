//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
#include "../../writer.hpp"
#include "../string_validation.hpp"
#include "args.hpp"
#include "parser.hpp"

namespace emio::detail::scan {

struct scan_trait {
  template <typename... Args>
  [[nodiscard]] static constexpr bool validate_string(std::string_view spec_str) {
    if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
      return validate<scan_specs_checker>(spec_str, sizeof...(Args), std::type_identity<Args>{}...);
    } else {
      return validate<scan_specs_checker>(spec_str, sizeof...(Args),
                                          make_validation_args<scan_validation_arg, Args...>(spec_str));
    }
  }
};

template <typename... Args>
using scan_string = validated_string<scan_trait, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_scan_string = valid_string<scan_trait, std::type_identity_t<Args>...>;

inline result<void> vscan_from(reader& in, const scan_args& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_str());
  return parse<scan_parser>(str, in, args);
}

template <typename... Args>
constexpr result<void> scan_from(reader& in, scan_string<Args...> scan_string, Args&... args) noexcept {
  EMIO_TRY(const std::string_view str, scan_string.get());
  return parse<scan_parser>(str, in, args...);
}

}  // namespace emio::detail::scan
