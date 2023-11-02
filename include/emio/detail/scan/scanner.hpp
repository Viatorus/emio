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
  default:
    EMIO_Z_DEV_ASSERT(false);
    EMIO_Z_INTERNAL_UNREACHABLE;
  }
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
constexpr result<void> read_arg(reader& original_in, const format_specs& specs, Arg& arg) noexcept {
  reader in = original_in;
  if (specs.width != no_width) {
    if (in.cnt_remaining() < static_cast<size_t>(specs.width)) {
      return err::eof;
    }
    EMIO_TRY(in, in.subreader(0, static_cast<size_t>(specs.width)));
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
    if (!in.eof()) {
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
constexpr result<void> read_arg(reader& in, const format_specs& /*unused*/, Arg& arg) noexcept {
  EMIO_TRY(arg, in.read_char());
  return success;
}

inline constexpr result<void> read_string_complex(reader& in, const std::string_view format_str,
                                                  std::string_view& arg) noexcept {
  // The following algorithm compares the chars of the format string (`format`) against the chars of the input string
  // (`in`).
  // The chars are compared one by one (#1).
  // The `format` contains at least one escape sequence of '{{' or '}}', therefor, at least one char in `format` must be
  // skipped (#2).
  // If there is a missmatch, the chars of `format` starts from the beginning but `in` remains unchanged (#3).
  // The algorithm ends successfully if:
  // - all chars of `format` are found inside `in` (#4)
  // - chars in `format` are found inside `in` and the next chars in `format` is another replacement field (#5)
  // The algorithm terminates without success if all chars of `in` has been compared (#6).

  const char* const format_begin = format_str.begin();
  const char* format_it = format_begin;
  const char* const format_end = format_str.end();

  const std::string_view in_remaining = in.view_remaining();
  const char* const in_begin = in_remaining.begin();
  const char* in_it = in_begin;
  const char* const in_end = in_remaining.end();

  size_t matches_cnt = 0;  // Count number matches.
  while (true) {
    if (format_it == format_end) {
      break;  // Complete spec matches input. Succeed. #4
    }
    if (in_it == in_end) {
      return err::invalid_data;  // Reached end of input. Fail. #6
    }

    // If there is an escape sequence, skip one spec char. #2
    if (*format_it == '{') {
      EMIO_Z_DEV_ASSERT((format_it + 1) != format_end);  // Spec is already validated.
      if (*(format_it + 1) != '{') {                     // New replacement field.
        break;                                           // Spec matches input. Succeed. #5
      }
      format_it += 1;                        // Skip escaped one.
      EMIO_Z_DEV_ASSERT(*format_it == '{');  // Must be '{'.
    } else if (*format_it == '}') {
      EMIO_Z_DEV_ASSERT((format_it + 1) != format_end);  // Spec is already validated.
      format_it += 1;                                    // Skip escaped one.
      EMIO_Z_DEV_ASSERT(*format_it == '}');              // Must be '}'.
    }

    if (*in_it == *format_it) {  // If input and spec match, check next spec char. #1
      ++format_it;
      ++matches_cnt;
    } else {  // Otherwise start from the beginning with the spec. #3
      format_it = format_begin;
      matches_cnt = 0;
    }
    ++in_it;
  }
  // `in` and `format` matches. Capture string.
  arg = std::string_view{in_begin, in_it - matches_cnt};
  in.pop(arg.size());
  return success;
}

inline constexpr result<void> read_string(reader& in, format_specs& specs, reader& format_rdr,
                                          std::string_view& arg) noexcept {
  // There exists 5 cases on how to read a string.
  // 1) The string spec has specified an exact width.
  // 2) The remaining string spec is empty, read everything.
  // 3) The remaining string spec does not contain any possible escape sequence ('{{' or '}}'), read until match.
  // 4) The remaining string spec does contain a possible escape sequence, but it turns out, it is the replacement
  //    field.
  // 5) The remaining string spec does contain at least one escape sequence.

  if (specs.width != no_width) {  // 1)
    EMIO_TRY(arg, in.read_n_chars(static_cast<size_t>(specs.width)));
    return success;
  }
  const result<std::string_view> until_next_res = format_rdr.read_until_any_of("{}", {.keep_delimiter = true});
  if (until_next_res == err::eof) {  // Read everything. 2)
    arg = in.read_remaining();
    return success;
  }

  const result<char> next_char_res = format_rdr.read_char();
  const auto is_replacement_field = [&]() noexcept {  // 4)
    const char next_char = next_char_res.assume_value();
    const char over_next_char = format_rdr.read_char().assume_value();  // Spec is validated.
    return next_char == '{' && over_next_char != '{';
  };

  if (next_char_res == err::eof /* 3) */ || is_replacement_field()) {
    EMIO_TRY(arg, in.read_until_str(until_next_res.assume_value(), {.keep_delimiter = true}));
    return success;
  }
  format_rdr.unpop(2);                                               // Undo replacement field check from 4).
  return read_string_complex(in, format_rdr.view_remaining(), arg);  // 5)
}

//
// Checks.
//

// specs is passed by reference instead as return type to reduce copying of big value (and code bloat)
inline constexpr result<void> validate_format_specs(reader& format_rdr, format_specs& specs) noexcept {
  EMIO_TRY(char c, format_rdr.read_char());
  if (c == '}') {  // Scan end.
    return success;
  }

  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (isdigit(c)) {  // Width.
    format_rdr.unpop();
    EMIO_TRY(const uint32_t size, format_rdr.parse_int<uint32_t>());
    if (size == 0 || size > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.width = static_cast<int32_t>(size);
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (detail::isalpha(c)) {
    specs.type = c;
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (c == '}') {  // Scan end.
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> parse_format_specs(reader& format_rdr, format_specs& specs) noexcept {
  char c = format_rdr.read_char().assume_value();
  if (c == '}') {  // Scan end.
    return success;
  }

  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    c = format_rdr.read_char().assume_value();
  }
  if (isdigit(c)) {  // Width.
    format_rdr.unpop();
    specs.width = static_cast<int32_t>(format_rdr.parse_int<uint32_t>().assume_value());
    c = format_rdr.read_char().assume_value();
  }
  if (detail::isalpha(c)) {
    specs.type = c;
    format_rdr.pop();  // format_rdr.read_char() in validate_format_specs;
  }
  return success;
}

inline constexpr result<void> check_char_specs(const format_specs& specs) noexcept {
  if ((specs.type != no_type && specs.type != 'c') || specs.alternate_form || specs.width > 1) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_integral_specs(const format_specs& specs) noexcept {
  switch (specs.type) {
  case no_type:
  case 'b':
  case 'd':
  case 'o':
  case 'x':
    return success;
  default:
    return err::invalid_format;
  }
}

inline constexpr result<void> check_string_specs(const format_specs& specs) noexcept {
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
concept has_static_validate_function_v = requires { &scanner<T>::validate; };

template <typename T>
concept has_member_validate_function_v = requires { std::declval<scanner<T>>().validate(std::declval<reader&>()); };

template <typename T>
concept has_any_validate_function_v =
    has_static_validate_function_v<T> || std::is_member_function_pointer_v<decltype(&scanner<T>::validate)> ||
    has_member_validate_function_v<T>;

template <typename T>
inline constexpr bool is_core_type_v =
    std::is_same_v<T, char> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t>;

}  // namespace detail::scan

}  // namespace emio
