//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstdint>

namespace emio::detail::format {

// "{" [arg_id] [":" (format_spec)]

// replacement_field ::=  "{" [arg_id] [":" (format_spec)] "}"
// arg_id            ::=  integer
// integer           ::=  digit+
// digit             ::=  "0"..."9"

// format_spec ::=  [[fill]align][sign]["#"]["0"][width]["." precision]["L"][type]
// fill        ::=  <a character other than '{' or '}'>
// align       ::=  "<" | ">" | "^"
// sign        ::=  "+" | "-" | " "
// width       ::=  integer (<=int max)
// precision   ::=  integer (<=int max)
// type        ::=  "a" | "A" | "b" | "B" | "c" | "d" | "e" | "E" | "f" | "F" | "g" | "G"| "o" | "O" | "p" | "s" | "x"
//                  | "X"

inline constexpr char no_sign = '\0';
inline constexpr int no_precision = -1;
inline constexpr char no_type = 0;

enum class alignment : uint8_t { none, left, center, right };

struct format_specs {
  char fill{' '};
  alignment align{alignment::none};
  char sign{no_sign};
  bool alternate_form{false};
  bool zero_flag{false};
  int32_t width{0};
  int32_t precision{no_precision};
  char type{no_type};
};

}  // namespace emio::detail::format
