//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>

#include "conversion.hpp"

namespace emio::detail {

constexpr bool needs_escape(uint32_t cp) {
  return cp < 0x20 || cp >= 0x7f || cp == '\'' || cp == '"' || cp == '\\';
}

template <typename Char, typename OutputIt>
constexpr OutputIt write_escaped(std::basic_string_view<Char> sv, OutputIt out) {
  for (Char c : sv) {
    if (!needs_escape(static_cast<uint32_t>(c))) {
      *(out++) = c;
    } else {
      switch (c) {
      case '\n':
        *(out++) = '\\';
        *(out++) = 'n';
        break;
      case '\r':
        *(out++) = '\\';
        *(out++) = 'r';
        break;
      case '\t':
        *(out++) = '\\';
        *(out++) = 't';
        break;
      case '\\':
        *(out++) = '\\';
        *(out++) = '\\';
        break;
      case '\'':
        *(out++) = '\\';
        *(out++) = '\'';
        break;
      case '"':
        *(out++) = '\\';
        *(out++) = '"';
        break;
      default: {
        // Escape char zero filled like: \x05, \x0ABC, \x00ABCDEF
        *(out++) = '\\';
        *(out++) = 'x';
        const auto abs = detail::to_absolute(detail::to_unsigned(c));
        const size_t number_of_digits = count_digits<16>(abs);
        // Fill up with zeros.
        for (size_t i = 0; i < 2 * sizeof(Char) - number_of_digits; i++) {
          *(out++) = '0';
        }
        out += to_signed(number_of_digits);
        write_number(abs, 16, false, out);
        break;
      }
      }
    }
  }
  return out;
}

template <typename Char>
constexpr size_t count_size_when_escaped(std::basic_string_view<Char> sv) {
  size_t count = 0;
  for (Char c : sv) {
    if (!needs_escape(static_cast<uint32_t>(c))) {
      count += 1;
    } else if (c == '\n' || c == '\r' || c == '\t' || c == '\\' || c == '\'' || c == '"') {
      count += 2;
    } else {
      count += 2 + 2 * sizeof(Char);  // \xAB...
    }
  }
  return count;
}

}  // namespace emio::detail
