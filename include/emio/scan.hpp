//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/scan/scan_from.hpp"

namespace emio {

/**
 * Provides access to the scan string and the arguments to scan.
 * @note This type should only be "constructed" via make_scan_args(scan_str, args...) and passed directly to a
 * scanning function.
 */
using scan_args = detail::args_span<detail::scan::scan_arg>;

// Alias template types.
template <typename... Args>
using scan_string = detail::scan::scan_string<Args...>;

template <typename... Args>
using valid_scan_string = detail::scan::valid_scan_string<Args...>;

/**
 * Returns an object that stores a scan string with an array of all arguments to scan.

 * @note The storage uses reference semantics and does not extend the lifetime of args. It is the programmer's
 * responsibility to ensure that args outlive the return value. Usually, the result is only used as argument to a
 * scanning function taking scan_args by reference.

 * @param scan_str The scan string.
 * @param args The arguments to be scanned.
 * @return Internal type. Implicit convertible to scan_args.
 */
template <typename... Args>
[[nodiscard]] detail::args_storage<detail::scan::scan_arg, sizeof...(Args)> make_scan_args(
    scan_string<Args...> scan_str, Args&... args) noexcept {
  return {scan_str.get(), args...};
}

/**
 * Scans the content of the reader for the given arguments according to the scan string.
 * @param rdr The reader to scan.
 * @param args The scan args with scan string.
 * @return Success if the scanning was successfully for all arguments. The reader may not be empty.
 */
inline result<void> vscan_from(reader& rdr, const scan_args& args) noexcept {
  return detail::scan::vscan_from(rdr, args);
}

/**
 * Scans the content of the input string for the given arguments according to the scan string.
 * @param input The input string to scan.
 * @param args The scan args with scan string.
 * @return Success if the scanning was successfully for all arguments for the entire input string.
 */
inline result<void> vscan(std::string_view input, const scan_args& args) noexcept {
  reader rdr{input};
  EMIO_TRYV(detail::scan::vscan_from(rdr, args));
  if (rdr.eof()) {
    return success;
  }
  return err::invalid_format;
}

/**
 * Scans the content of the reader for the given arguments according to the scan string.
 * @param rdr The reader.
 * @param scan_string The scan string.
 * @param args The arguments which are to be scanned.
 * @return Success if the scanning was successfully for all arguments. The reader may not be empty.
 */
template <typename... Args>
constexpr result<void> scan_from(reader& rdr, scan_string<Args...> scan_string, Args&... args) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::scan::scan_from(rdr, scan_string, args...));
  } else {
    EMIO_TRYV(detail::scan::vscan_from(rdr, make_scan_args(scan_string, args...)));
  }
  return success;
}

/**
 * Scans the input string for the given arguments according to the scan string.
 * @param input The input string.
 * @param scan_string The scan string.
 * @param args The arguments which are to be scanned.
 * @return Success if the scanning was successfully for all arguments for the entire input string.
 */
template <typename... Args>
constexpr result<void> scan(std::string_view input, scan_string<Args...> scan_string, Args&... args) {
  reader rdr{input};
  EMIO_TRYV(scan_from(rdr, scan_string, args...));
  if (rdr.eof()) {
    return success;
  }
  return err::invalid_format;
}

}  // namespace emio
