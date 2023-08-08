//
// Copyright (c) 2021 - present, Toni Neubert
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

inline result<void> vscan_from(reader& input, const args_span<scan_arg>& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_str());
  return parse<scan_parser>(str, input, args);
}

template <typename... Args>
constexpr result<void> scan_from(reader& input, validated_string<scan_trait, Args...> scan_string,
                                 Args&... args) noexcept {
  EMIO_TRY(const std::string_view str, scan_string.get());
  return parse<scan_parser>(str, input, args...);
}

}  // namespace emio::detail::scan
