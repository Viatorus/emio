//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/scan/scan_from.hpp"

namespace emio {

/**
 * Provides access to the format string and the arguments to scan.
 * @note This type should only be "constructed" via make_scan_args(format_str, args...) and passed directly to a
 * scanning function.
 */
using scan_args = detail::scan::scan_args;

// Alias template types.
template <typename... Args>
using format_scan_string = detail::scan::format_string<Args...>;

template <typename... Args>
using valid_format_scan_string = detail::scan::valid_format_string<Args...>;

/**
 * Returns an object that stores a format string with an array of all arguments to scan.
 *
 * @note The storage uses reference semantics and does not extend the lifetime of args. It is the programmer's
 * responsibility to ensure that args outlive the return value. Usually, the result is only used as argument to a
 * scanning function taking scan_args by reference.
 *
 * @param format_str The format string.
 * @param args The arguments to be scanned.
 * @return Internal type. Implicit convertible to scan_args.
 */
template <typename... Args>
[[nodiscard]] detail::args_storage<detail::scan::scan_arg, sizeof...(Args)> make_scan_args(
    format_scan_string<Args...> format_str, Args&... args) noexcept {
  return {format_str, args...};
}

/**
 * Scans the content of the reader for the given arguments according to the format string.
 * @param in_rdr The reader to scan.
 * @param args The scan args with format string.
 * @return Success if the scanning was successfully for all arguments. The reader may not be empty.
 */
inline result<void> vscan_from(reader& in_rdr, const scan_args& args) noexcept {
  return detail::scan::vscan_from(in_rdr, args);
}

/**
 * Scans the content of the input string for the given arguments according to the format string.
 * @param in The input string to scan.
 * @param args The scan args with format string.
 * @return Success if the scanning was successfully for all arguments for the entire input string.
 */
inline result<void> vscan(std::string_view in, const scan_args& args) noexcept {
  reader in_rdr{in};
  EMIO_TRYV(detail::scan::vscan_from(in_rdr, args));
  if (in_rdr.eof()) {
    return success;
  }
  return err::invalid_format;
}

/**
 * Scans the content of the reader for the given arguments according to the format string.
 * @param in_rdr The reader to scan.
 * @param format_str The format string.
 * @param args The arguments which are to be scanned.
 * @return Success if the scanning was successfully for all arguments. The reader may not be empty.
 */
template <typename... Args>
constexpr result<void> scan_from(reader& in_rdr, format_scan_string<Args...> format_str, Args&... args) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::scan::scan_from(in_rdr, format_str, args...));
  } else {
    EMIO_TRYV(detail::scan::vscan_from(in_rdr, make_scan_args(format_str, args...)));
  }
  return success;
}

/**
 * Scans the input string for the given arguments according to the format string.
 * @param input The input string.
 * @param format_str The format string.
 * @param args The arguments which are to be scanned.
 * @return Success if the scanning was successfully for all arguments for the entire input string.
 */
template <typename... Args>
constexpr result<void> scan(std::string_view input, format_scan_string<Args...> format_str, Args&... args) noexcept {
  reader rdr{input};
  EMIO_TRYV(emio::scan_from(rdr, format_str, args...));
  if (rdr.eof()) {
    return success;
  }
  return err::invalid_format;
}

}  // namespace emio
