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
    if constexpr (std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>) {
      using upcast_int_t = decltype(detail::integer_upcast(Arg{}));
      if constexpr (std::is_same_v<upcast_int_t, Arg>) {
        scanner<Arg> scanner;
        EMIO_TRYV(scanner.parse(format_rdr));
        return scanner.scan(in, arg);
      } else {
        // Check if upcast int is within the integer type range.
        upcast_int_t val{};
        EMIO_TRYV(scan_arg_trait<upcast_int_t>::process_arg(in, format_rdr, val));
        if (val < std::numeric_limits<Arg>::min() || val > std::numeric_limits<Arg>::max()) {
          return err::out_of_range;
        }
        arg = static_cast<Arg>(val);
        return success;
      }
    } else {
      scanner<Arg> scanner;
      EMIO_TRYV(scanner.parse(format_rdr));
      return scanner.scan(in, arg);
    }

  }
};

using scan_validation_arg = validation_arg<scan_arg_trait>;

using scan_arg = arg<reader, scan_arg_trait>;

using scan_args = args_span<scan_arg>;

}  // namespace emio::detail::scan
