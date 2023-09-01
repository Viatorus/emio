//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
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

template <typename T>
inline constexpr bool is_core_type_v = std::is_integral_v<T>;

}  // namespace detail::scan

}  // namespace emio
