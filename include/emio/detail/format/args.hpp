//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../formatter.hpp"
#include "../args.hpp"

namespace emio::detail::format {

template <typename Arg>
struct format_arg_trait {
  using unified_type = format::unified_type_t<std::remove_const_t<Arg>>;

  static constexpr result<void> validate(reader& format_rdr) noexcept {
    // Check if a formatter exist and a correct validate method is implemented. If not, use the parse method.
    if constexpr (has_formatter_v<Arg>) {
      if constexpr (has_validate_function_v<Arg>) {
        return formatter<Arg>::validate(format_rdr);
      } else {
        static_assert(!has_any_validate_function_v<Arg>,
                      "Formatter seems to have a validate property which doesn't fit the desired signature.");
        return formatter<Arg>{}.parse(format_rdr);
      }
    } else {
      static_assert(has_formatter_v<Arg>,
                    "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
      return err::invalid_format;
    }
  }

  static constexpr result<void> process_arg(writer& out, reader& format_rdr, const Arg& arg) noexcept {
    formatter<Arg> formatter;
    EMIO_TRYV(formatter.parse(format_rdr));
    return formatter.format(out, arg);
  }
};

using format_validation_arg = validation_arg<format_arg_trait>;

using format_arg = arg<writer, format_arg_trait>;

using format_args = args_span_with_str<format_arg>;

}  // namespace emio::detail::format
