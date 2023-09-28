//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../scanner.hpp"
#include "../args.hpp"

namespace emio::detail::scan {

template <typename Arg>
struct scan_arg_trait {
  using unified_type = Arg&;

  static constexpr result<void> validate(reader& format_rdr) noexcept {
    // Check if a scanner exist and a correct validate method is implemented. If not, use the parse method.
    if constexpr (has_scanner_v<Arg>) {
      if constexpr (has_validate_function_v<Arg>) {
        return scanner<Arg>::validate(format_rdr);
      } else {
        static_assert(!has_any_validate_function_v<Arg>,
                      "Scanner seems to have a validate property which doesn't fit the desired signature.");
        return scanner<Arg>{}.parse(format_rdr);
      }
    } else {
      static_assert(has_scanner_v<Arg>,
                    "Cannot scan an argument. To make type T scannable provide a scanner<T> specialization.");
      return err::invalid_format;
    }
  }

  static constexpr result<void> process_arg(reader& in, reader& format_rdr, Arg& arg) noexcept {
    scanner<Arg> scanner;
    EMIO_TRYV(scanner.parse(format_rdr));
    return scanner.scan(in, arg);
  }
};

using scan_validation_arg = validation_arg<scan_arg_trait>;

using scan_arg = arg<reader, scan_arg_trait>;

using scan_args = args_span_with_str<scan_arg>;

}  // namespace emio::detail::scan
