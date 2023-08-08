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

namespace emio::detail::format {

// Non constexpr version.
inline result<void> vformat_to(buffer& buf, const args_span<format_arg>& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_str());
  writer wtr{buf};
  return parse<format_parser>(str, wtr, args);
}

// Constexpr version.
template <typename... Args>
constexpr result<void> format_to(buffer& buf, validated_string<format_trait, Args...> format_str,
                                 const Args&... args) noexcept {
  EMIO_TRY(const std::string_view str, format_str.get());
  writer wtr{buf};
  return parse<format_parser>(str, wtr, args...);
}

}  // namespace emio::detail::format
