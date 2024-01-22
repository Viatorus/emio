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
    return detail::format::validate_trait<Arg>(format_rdr);
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
