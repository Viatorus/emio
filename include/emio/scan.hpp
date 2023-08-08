//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/scan/scan_from.hpp"

namespace emio {

// Alias template types.
using scan_args = detail::args_span<detail::scan::scan_arg>;  // TODO vscan

template <typename... Args>
using scan_string = detail::validated_string<detail::scan::scan_trait, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_scan_string = detail::valid_string<detail::scan::scan_trait, std::type_identity_t<Args>...>;

template <typename... Args>
[[nodiscard]] detail::args_storage<detail::scan::scan_arg, sizeof...(Args)> make_scan_args(
    scan_string<Args...> scan_str, Args&... args) noexcept {
  return {scan_str.get(), args...};
}

template <typename... Args>
constexpr result<void> scan_from(reader& rdr, scan_string<Args...> scan_string, Args&... args) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::scan::scan_from(rdr, scan_string, args...));
  } else {
    EMIO_TRYV(detail::scan::vscan_from(rdr, make_scan_args(scan_string, args...)));
  }
  return success;
}

template <typename... Args>
constexpr result<void> scan(std::string_view input, scan_string<Args...> scan_string, Args&... args) {
  reader rdr{input};
  return scan_from(rdr, scan_string, args...);
}

}  // namespace emio
