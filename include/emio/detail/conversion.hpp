//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <bit>
#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>
#include <type_traits>

namespace emio::detail {

constexpr bool isalpha(char c) {
  return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

constexpr bool isdigit(char c) {
  return (c >= '0' && c <= '9');
}

constexpr bool is_valid_number_base(int base) noexcept {
  return base >= 2 && base <= 36;
}

constexpr std::optional<int> char_to_digit(char c, int base) noexcept {
  if (c < '0') {
    return std::nullopt;
  }
  int res{};
  if (c >= 'a') {
    res = c - 'a' + 10;
  } else if (c >= 'A') {
    res = c - 'A' + 10;
  } else {
    res = c - '0';
  }
  if (res < base) {
    return res;
  }
  return std::nullopt;
}

constexpr char digit_to_char(int digit, int base, bool upper) noexcept {
  if (base >= 1 && digit >= 10) {
    if (upper) {
      return static_cast<char>(static_cast<int>('A') + (digit - 10));
    }
    return static_cast<char>(static_cast<int>('a') + (digit - 10));
  }
  return static_cast<char>(static_cast<int>('0') + digit);
}

template <typename T>
constexpr size_t count_digits_10(T number) noexcept {
  size_t count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++".
    if (number < 10) {
      return count;
    }
    if (number < 100) {
      return count + 1;
    }
    if (number < 1000) {
      return count + 2;
    }
    if (number < 10000) {
      return count + 3;
    }
    number /= 10000U;
    count += 4;
  }
}

template <size_t Base, typename T>
constexpr size_t count_digits(T number) noexcept {
  if (number == 0) {
    return 1;
  }

  if constexpr (Base == 10) {
    return count_digits_10(number);
  } else if constexpr (Base == 2) {
    return std::bit_width(number);
  } else if constexpr (Base == 8) {
    return (std::bit_width(number) + 2) / 3;
  } else if constexpr (Base == 16) {
    return (std::bit_width(number) + 3) / 4;
  } else {
    size_t digit_cnt{1};
    for (number /= static_cast<T>(Base); number; number /= static_cast<T>(Base)) {
      ++digit_cnt;
    }
    return digit_cnt;
  }
}

template <typename T>
  requires(std::is_unsigned_v<T>)
constexpr size_t get_number_of_digits(T number, int base) noexcept {
  if (number == 0) {
    return 1;
  }
  if (base == 10) {
    return count_digits<10>(number);
  }
  if (base == 16) {
    return count_digits<16>(number);
  }
  if (base == 2) {
    return count_digits<2>(number);
  }
  if (base == 8) {
    return count_digits<8>(number);
  }
  size_t digit_cnt{1};
  for (number /= static_cast<T>(base); number; number /= static_cast<T>(base)) {
    ++digit_cnt;
  }
  return digit_cnt;
}

template <typename T>
constexpr bool is_negative(T value) noexcept {
  if constexpr (std::is_signed_v<T>) {
    return value < 0;
  } else {
    return false;
  }
}

template <typename T>
constexpr int num_bits() noexcept {
  return std::numeric_limits<T>::digits;
}

template <typename T>
using int32_or_64 = std::conditional_t<num_bits<T>() <= 32, int32_t, int64_t>;

template <typename T>
using uint32_or_64 = std::conditional_t<num_bits<T>() <= 32, uint32_t, uint64_t>;

template <typename T>
  requires(std::is_integral_v<T>)
constexpr auto integer_upcast(T integer) {
  if constexpr (std::is_signed_v<T>) {
    return static_cast<int32_or_64<T>>(integer);
  } else {
    return static_cast<uint32_or_64<T>>(integer);
  }
}

template <typename T>
constexpr uint32_or_64<T> to_absolute(T number) noexcept {
  if constexpr (std::is_unsigned_v<T>) {
    return number;
  } else {
    if (is_negative(number)) {
      auto abs = static_cast<uint32_or_64<T>>(number);
      abs = T{} - abs;
      return abs;
    }
    return static_cast<uint32_or_64<T>>(number);
  }
}

template <typename T>
constexpr std::make_unsigned_t<T> to_unsigned(T number) noexcept {
  return static_cast<std::make_unsigned_t<T>>(number);
}

template <typename T>
constexpr std::make_signed_t<T> to_signed(T number) noexcept {
  return static_cast<std::make_signed_t<T>>(number);
}

template <typename T, typename OutputIt>
  requires(std::is_unsigned_v<T>)
constexpr OutputIt write_number(T abs_number, int base, bool upper, OutputIt next) {
  if (abs_number == 0) {
    *(--next) = '0';  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic): performance
    return next;
  }
  // Write number from right to left.
  for (; abs_number; abs_number /= static_cast<T>(base)) {
    const char c = digit_to_char(static_cast<int>(abs_number % static_cast<T>(base)), base, upper);
    *(--next) = c;  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic): performance
  }
  return next;
}

inline constexpr size_t npos = std::string_view::npos;

template <typename Char>
constexpr std::basic_string_view<Char> unchecked_substr(const std::basic_string_view<Char>& str, size_t pos,
                                                        size_t n = npos) noexcept {
  const size_t rlen = std::min(n, str.length() - pos);
  return {str.data() + pos, rlen};
}

}  // namespace emio::detail