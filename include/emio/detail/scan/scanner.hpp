//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
#include "../../writer.hpp"
#include "../parser.hpp"
#include "specs.hpp"

namespace emio {

template <typename>
class scanner;

namespace detail::scan {

//
// Read args.
//

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
result<void> read_arg(reader& in, const scan_specs& /*unused*/, Arg& arg) {
  EMIO_TRY(arg, in.parse_int<Arg>());
  return success;
}

template <typename Arg>
  requires(std::is_same_v<Arg, char>)
result<void> read_arg(reader& in, const scan_specs& /*unused*/, Arg& arg) {
  EMIO_TRY(arg, in.read_char());
  return success;
}

// Specifies if T has an enabled formatter specialization.
template <typename Arg>
inline constexpr bool has_scanner_v = std::is_constructible_v<scanner<Arg>>;

template <typename T>
concept has_validate_function_v = requires {
  { scanner<T>::validate(std::declval<reader&>()) } -> std::same_as<result<void>>;
};

template <typename T>
concept has_any_validate_function_v =
    requires { &scanner<T>::validate; } || std::is_member_function_pointer_v<decltype(&scanner<T>::validate)> ||
    requires { std::declval<scanner<T>>().validate(std::declval<reader&>()); };

template <typename Arg>
constexpr result<void> validate_for(reader& format_is) noexcept {
  // Check if a scanner exist and a correct validate method is implemented. If not, use the parse method.
  if constexpr (has_scanner_v<Arg>) {
    if constexpr (has_validate_function_v<Arg>) {
      return scanner<Arg>::validate(format_is);
    } else {
      static_assert(!has_any_validate_function_v<Arg>,
                    "Scanner seems to have a validate property which doesn't fit the desired signature.");
      return scanner<Arg>{}.parse(format_is);
    }
  } else {
    static_assert(has_scanner_v<Arg>,
                  "Cannot format an argument. To make type T scannable provide a scanner<T> specialization.");
    return err::invalid_format;
  }
}

template <input_validation FormatStringValidation, typename T>
concept scanner_parse_supports_format_string_validation =
    requires(T scanner) { scanner.template parse<FormatStringValidation>(std::declval<reader>()); };

template <input_validation FormatStringValidation, typename T>
inline constexpr result<void> invoke_scanner_parse(T& scanner, reader& scan_is) noexcept {
  if constexpr (scanner_parse_supports_format_string_validation<FormatStringValidation, T>) {
    return scanner.template parse<FormatStringValidation>(scan_is);
  } else {
    return scanner.parse(scan_is);
  }
}

}  // namespace detail::scan

}  // namespace emio