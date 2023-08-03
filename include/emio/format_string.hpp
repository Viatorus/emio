//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>
#include <type_traits>

#include "detail/format/format_string.hpp"
#include "detail/scan/scan_string.hpp"

namespace emio {

// Alias template types.
using runtime_format_string = detail::format::runtime_format_string;

template <typename... Args>
using format_string = detail::format::format_string<std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = detail::format::valid_format_string<std::type_identity_t<Args>...>;

inline constexpr runtime_format_string runtime(const std::string_view& s) noexcept {
  return runtime_format_string{s};
}

template <typename... Args>
using scan_string = detail::scan::scan_string<std::type_identity_t<Args>...>;

}  // namespace emio
