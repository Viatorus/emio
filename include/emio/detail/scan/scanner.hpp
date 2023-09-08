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

inline constexpr result<void> disallow_sign(reader& in) noexcept {
  EMIO_TRY(const char c, in.peek());
  if (c == '+' || c == '-') {
    return err::invalid_data;
  }
  return success;
}

inline constexpr result<int> determine_base(reader in) noexcept {
  EMIO_TRY(char c, in.read_char());
  if (c != '0') {
    // Assume decimal.
    return 10;
  }
  result<char> next_char = in.read_char();
  if (!next_char) {
    // Assume decimal.
    return 10;
  }
  c = next_char.assume_value();
  if (c == 'b' || c == 'B') {
    return 2;
  }
  if (c == 'x' || c == 'X') {
    return 16;
  }
  return 8;
}

inline constexpr int get_base(char type) noexcept {
  switch (type) {
  case 'b':
    return 2;
  case 'o':
    return 8;
  case 'd':
  case no_type:
    return 10;
  case 'x':
    return 16;
  }
  EMIO_Z_DEV_ASSERT(false);
  EMIO_Z_INTERNAL_UNREACHABLE;
}

inline constexpr result<void> parse_alternate_form(reader& in, int base) noexcept {
  EMIO_TRYV(in.read_if_match_char('0'));

  if (base == 8) {
    return success;
  }
  EMIO_TRY(const char c, in.read_char());

  if (base == 2) {
    if (c == 'b' || c == 'B') {
      return success;
    }
    return err::invalid_data;
  }

  // Base 16.
  if (c == 'x' || c == 'X') {
    return success;
  }
  return err::invalid_data;
}

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
constexpr result<void> read_arg(reader& original_in, const scan_specs& specs, Arg& arg) noexcept {
  reader in = original_in;
  if (specs.width != no_width) {
    EMIO_TRY(std::string_view sub_content, in.read_n_chars(static_cast<size_t>(specs.width)));
    in = reader{sub_content};  // TODO TRIM?
  }

  EMIO_TRY(const bool is_negative, parse_sign(in));

  int base = 0;
  if (specs.type == no_type && specs.alternate_form) {
    EMIO_TRY(base, determine_base(in));
    if (base != 10) {
      // Discard alternate form from input.
      if (base == 8) {
        in.pop(1);
      } else {
        in.pop(2);
      }
      EMIO_TRYV(disallow_sign(in));
    }
  } else {
    base = get_base(specs.type);
    if (specs.alternate_form && base != 10) {
      EMIO_TRYV(parse_alternate_form(in, base));
      EMIO_TRYV(disallow_sign(in));
    }
  }
  EMIO_TRY(arg, parse_int<Arg>(in, base, is_negative));

  if (specs.width != no_width) {
    if (!in.view_remaining().empty()) {  // TODO: empty?
      return err::invalid_data;
    }
    original_in.pop(static_cast<size_t>(specs.width));
  } else {
    original_in = in;
  }
  return success;
}

template <typename Arg>
  requires(std::is_same_v<Arg, char>)
constexpr result<void> read_arg(reader& in, const scan_specs& /*unused*/, Arg& arg) noexcept {
  EMIO_TRY(arg, in.read_char());
  return success;
}

inline constexpr result<void> read_arg(reader& in, scan_string_specs& specs, std::string_view& arg) noexcept {
  if (specs.width != no_width) {
    EMIO_TRY(arg, in.read_n_chars(static_cast<size_t>(specs.width)));
    return success;
  }
  const result<std::string_view> until_next_res = specs.remaining.read_until_any_of("{}", {.keep_delimiter = true});
  if (until_next_res == err::eof) {  // Just read until end.
    arg = in.read_remaining();
    return success;
  }
  const auto r = [&] {
    const char next_char = specs.remaining.read_char().assume_value();
    const char overnext_char = specs.remaining.read_char().assume_value();  // Cannot be eof.
    return (next_char == '{' && overnext_char != '{');
  };

  if (specs.remaining.eof() || r()) {  // Just read until matches_cnt.
    EMIO_TRY(arg, in.read_until_str(until_next_res.assume_value(), {.keep_delimiter = true}));
    return success;
  }

  const std::string_view remaining = in.view_remaining();
  const char* const src_begin = remaining.begin();
  const char* src_it = src_begin;
  const char* const src_end = remaining.end();

  specs.remaining.unpop(2);
  const std::string_view spec_remaining = specs.remaining.view_remaining();
  const char* const dst_begin = spec_remaining.begin();
  const char* dst_it = dst_begin;
  const char* const dst_end = spec_remaining.end();

  size_t matches_cnt = 0;

  while (true) {
    if (dst_it == dst_end) {
      break;  // Complete spec matches input. Succeed.
    }
    if (src_it == src_end) {
      return err::invalid_data;  // Reached end of input. Fail.
    }

    // Check for maybe escaped { and }.
    // Skip them.
    if (*dst_it == '{') {                          // If + 1 wasn't there, it format wouldn't be validated.
      EMIO_Z_DEV_ASSERT((dst_it + 1) != dst_end);  // Spec is already validated.
      if (*(dst_it + 1) != '{') {                  // New replacement field.
        break;                                     // Spec matches input. Succeed.
      }
      dst_it += 1;
      EMIO_Z_DEV_ASSERT(*dst_it == '{');  // Must be '{'.
    } else if (*dst_it == '}') {
      EMIO_Z_DEV_ASSERT((dst_it + 1) != dst_end);  // Spec is already validated.
      dst_it += 1;
      EMIO_Z_DEV_ASSERT(*dst_it == '}');  // Must be '}'.
    }

    if (*src_it == *dst_it) {  // If input and spec match, check next spec char.
      ++dst_it;
      ++matches_cnt;
    } else {  // Otherwise start from the beginning with the spec.
      matches_cnt = 0;
      dst_it = dst_begin;
    }
    ++src_it;
  }
  // Found matches_cnt.
  arg = std::string_view{src_begin, src_it - matches_cnt};
  in.pop(arg.size());
  return success;
}

//
// Checks.
//

// specs is passed by reference instead as return type to reduce copying of big value (and code bloat)
inline constexpr result<void> validate_scan_specs(reader& rdr, scan_specs& specs) noexcept {
  EMIO_TRY(char c, rdr.read_char());
  if (c == '}') {  // Scan end.
    return success;
  }

  if (isdigit(c)) {  // Width.
    rdr.unpop();
    EMIO_TRY(const uint32_t size, rdr.parse_int<uint32_t>());
    if (size == 0 || size > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.width = static_cast<int32_t>(size);
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isalpha(c)) {
    specs.type = c;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '}') {  // Scan end.
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> parse_scan_specs(reader& rdr, scan_specs& specs) noexcept {
  char c = rdr.read_char().assume_value();
  if (c == '}') {  // Scan end.
    return success;
  }

  if (isdigit(c)) {  // Width.
    rdr.unpop();
    specs.width = static_cast<int32_t>(rdr.parse_int<uint32_t>().assume_value());
    c = rdr.read_char().assume_value();
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    c = rdr.read_char().assume_value();
  }
  if (detail::isalpha(c)) {
    specs.type = c;
    rdr.pop();  // rdr.read_char() in validate_scan_specs;
  }
  return success;
}

inline constexpr result<void> check_char_specs(const scan_specs& specs) noexcept {
  if ((specs.type != no_type && specs.type != 'c') || specs.alternate_form || specs.width > 1) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_integral_specs(const scan_specs& specs) noexcept {
  switch (specs.type) {
  case no_type:
  case 'b':
  case 'd':
  case 'o':
  case 'x':
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_string_specs(const scan_specs& specs) noexcept {
  if ((specs.type != no_type && specs.type != 's') || specs.alternate_form) {
    return err::invalid_format;
  }
  return success;
}

//
// Type traits.
//

// Specifies if T has an enabled scanner specialization.
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
inline constexpr bool is_core_type_v = !std::is_same_v<T, bool> && std::is_integral_v<T>;

}  // namespace detail::scan

}  // namespace emio
