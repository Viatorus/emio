//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/scan/scanner.hpp"

namespace emio {

template <typename T>
class scanner {
 public:
  static constexpr result<void> validate(reader& rdr) noexcept {
    EMIO_TRY(const char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> scan(reader& input, T& arg) const noexcept {
    return read_arg(input, specs_, arg);
  }

  detail::scan::scan_specs specs_{};
};

}  // namespace emio