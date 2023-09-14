//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

#include "../buffer.hpp"
#include "conversion.hpp"

namespace emio::detail {

inline constexpr bool needs_escape(uint32_t cp) noexcept {
  return cp < 0x20 || cp >= 0x7f || cp == '\'' || cp == '"' || cp == '\\';
}

inline constexpr size_t count_size_when_escaped(std::string_view sv) noexcept {
  size_t count = 0;
  for (const char c : sv) {
    if (!needs_escape(static_cast<uint32_t>(c))) {
      count += 1;
    } else if (c == '\n' || c == '\r' || c == '\t' || c == '\\' || c == '\'' || c == '"') {
      count += 2;
    } else {
      count += 2 + 2 * sizeof(char);  // \xAB...
    }
  }
  return count;
}

/*
 * Class which helps to escape a long string in smaller chunks.
 */
class write_escaped_helper {
 public:
  constexpr write_escaped_helper(std::string_view sv) noexcept : src_it_{sv.begin()}, src_end_{sv.end()} {}

  [[nodiscard]] constexpr size_t write_escaped(std::span<char> area) noexcept {
    char* dst_it = area.data();
    const char* const dst_end = area.data() + area.size();

    // Write remainder from temporary buffer.
    const auto write_remainder = [&, this]() noexcept {
      while (remainder_it_ != remainder_end_ && dst_it != dst_end) {
        *(dst_it++) = *(remainder_it_++);
      }
    };
    write_remainder();

    while (src_it_ != src_end_) {
      if (dst_it == dst_end) {
        return static_cast<size_t>(dst_it - area.data());
      }
      const char c = *src_it_++;
      if (!needs_escape(static_cast<uint32_t>(c))) {
        *(dst_it++) = c;
      } else {
        *(dst_it++) = '\\';
        const auto remaining_space = static_cast<size_t>(dst_end - dst_it);
        if (remaining_space >= 3) {
          dst_it = write_escaped(c, dst_it);
        } else {
          // Write escaped sequence to remainder.
          remainder_it_ = remainder_storage_.begin();
          remainder_end_ = write_escaped(c, remainder_it_);
          // Write as much as possible into dst.
          write_remainder();
        }
      }
    }
    return static_cast<size_t>(dst_it - area.data());
  }

 private:
  [[nodiscard]] static inline constexpr char* write_escaped(const char c, char* out) noexcept {
    switch (c) {
    case '\n':
      *(out++) = 'n';
      return out;
    case '\r':
      *(out++) = 'r';
      return out;
    case '\t':
      *(out++) = 't';
      return out;
    case '\\':
      *(out++) = '\\';
      return out;
    case '\'':
      *(out++) = '\'';
      return out;
    case '"':
      *(out++) = '"';
      return out;
    default: {
      // Escape char zero filled like: \x05
      *(out++) = 'x';
      const auto abs = detail::to_absolute(detail::to_unsigned(c));
      const size_t number_of_digits = count_digits<16>(abs);
      // Fill up with zeros.
      for (size_t i = 0; i < 2 * sizeof(char) - number_of_digits; i++) {
        *(out++) = '0';
      }
      out += to_signed(number_of_digits);
      write_number(abs, 16, false, out);
      return out;
    }
    }
  }

  const char* src_it_;  // Input to encode.
  const char* src_end_;
  std::array<char, 4> remainder_storage_{};  // Remainder containing data for the next iteration.
  char* remainder_it_{};
  char* remainder_end_{};
};

inline constexpr result<void> write_str_escaped(buffer& buf, std::string_view sv, size_t escaped_size,
                                                const char quote) {
  // Perform escaping in multiple chunks, to support buffers with an internal cache.
  detail::write_escaped_helper helper{sv};
  EMIO_TRY(auto area, buf.get_write_area_of_max(escaped_size + 2 /*both quotes*/));
  // Start quote.
  area[0] = quote;
  area = area.subspan(1);

  while (true) {
    const size_t written = helper.write_escaped(area);
    escaped_size -= written;
    if (escaped_size == 0) {
      area = area.subspan(written);
      break;
    }
    EMIO_TRY(area, buf.get_write_area_of_max(escaped_size + 1 /*end quote*/));
  }
  if (area.empty()) {
    EMIO_TRY(area, buf.get_write_area_of_max(1 /*end quote*/));
  }
  // End quote.
  area[0] = quote;
  return success;
}

}  // namespace emio::detail
