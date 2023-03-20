//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../writer.hpp"
#include "../parser.hpp"
#include "specs.hpp"

namespace emio {

template <typename>
class formatter;

namespace detail::format {

//
// Write args.
//

inline constexpr result<void> write_padding_left(writer<char>& wtr, format_specs& specs, size_t width) {
  if (specs.width == 0 || specs.width < static_cast<int>(width)) {
    specs.width = 0;
    return success;
  }
  int fill_width = specs.width - static_cast<int>(width);
  if (specs.align == alignment::left) {
    specs.width = fill_width;
    return success;
  }
  if (specs.align == alignment::center) {
    fill_width = fill_width / 2;
  }
  specs.width -= fill_width + static_cast<int>(width);
  return wtr.write_char_n(specs.fill, static_cast<size_t>(fill_width));
}

inline constexpr result<void> write_padding_right(writer<char>& wtr, format_specs& specs) {
  if (specs.width == 0 || (specs.align != alignment::left && specs.align != alignment::center)) {
    return success;
  }
  return wtr.write_char_n(specs.fill, static_cast<size_t>(specs.width));
}

template <alignment DefaultAlign, typename Func>
constexpr result<void> write_padded(writer<char>& wtr, format_specs& specs, size_t width, Func&& func) {
  if (specs.align == alignment::none) {
    specs.align = DefaultAlign;
  }
  EMIO_TRYV(write_padding_left(wtr, specs, width));
  EMIO_TRYV(func());
  return write_padding_right(wtr, specs);
}

inline constexpr result<std::pair<std::string_view, writer<char>::write_int_options>> make_write_int_options(
    char spec_type) noexcept {
  using namespace alternate_form;

  std::string_view prefix;
  writer<char>::write_int_options options{};

  switch (spec_type) {
  case no_type:
  case 'd':
    options.base = 10;
    break;
  case 'x':
    prefix = hex_lower;
    options = {.base = 16};
    break;
  case 'X':
    prefix = hex_upper;
    options = {.base = 16, .upper_case = true};
    break;
  case 'b':
    prefix = bin_lower;
    options.base = 2;
    break;
  case 'B':
    prefix = bin_upper;
    options.base = 2;
    break;
  case 'o':
    prefix = octal;
    options.base = 8;
    break;
  default:
    return err::invalid_format;
  }
  return std::pair{prefix, options};
}

inline constexpr result<void> write_sign_and_prefix(writer<char>& wtr, const format_specs& specs, bool negative,
                                                    std::string_view prefix) {
  if (negative) {
    EMIO_TRYV(wtr.write_char('-'));
  } else if (specs.sign == '+' || specs.sign == ' ') {
    EMIO_TRYV(wtr.write_char(specs.sign));
  }
  if (specs.alternate_form && !prefix.empty()) {
    EMIO_TRYV(wtr.write_str(prefix));
  }
  return success;
}

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const Arg& arg) noexcept {
  if (specs.type == 'c') {
    return write_padded<alignment::left>(wtr, specs, 1, [&] {
      return wtr.write_char(static_cast<char>(arg));
    });
  }
  EMIO_TRY((auto [prefix, options]), make_write_int_options(specs.type));

  if (specs.type == 'o' && arg == 0) {
    prefix = "";
  }

  const auto abs_number = detail::to_absolute(arg);
  const bool negative = detail::is_negative(arg);
  const size_t number_of_digits = detail::get_number_of_digits(abs_number, options.base);

  size_t total_length = number_of_digits;
  if (specs.alternate_form) {
    total_length += prefix.size();
  }
  if (negative || specs.sign == ' ' || specs.sign == '+') {
    total_length += 1;
  }

  if (specs.zero_flag) {
    EMIO_TRYV(write_sign_and_prefix(wtr, specs, negative, prefix));
  }

  return write_padded<alignment::right>(
      wtr, specs, total_length, [&, &prefix = prefix, &options = options]() -> result<void> {
        if (!specs.zero_flag) {
          EMIO_TRYV(write_sign_and_prefix(wtr, specs, negative, prefix));
        }

        auto& buf = wtr.get_buffer();
        EMIO_TRY(auto area, buf.get_write_area_of(number_of_digits));
        write_number(abs_number, options.base, options.upper_case, area.begin() + detail::to_signed(number_of_digits));
        return success;
      });
}

// template <typename Arg>
// requires(std::is_floating_point_v<Arg>)
//      result<void> write_arg(writer<char>& wtr, const format_specs&
//     specs,
//                                          const Arg& arg) noexcept {
//	result<void> err;
//	std::chars_format fmt;
//	switch (specs.type) {
//	case no_type:
//	case 'f':
//		fmt = std::chars_format::fixed;
//		break;
//		//	case 'G': upper
//	case 'g':
//		fmt = std::chars_format::general;
//		break;
//		//	case 'E': upper
//	case 'e':
//		fmt = std::chars_format::scientific;
//		break;
//		//	case 'F': upper
//		// case 'A': upper
//	case 'a':
//		fmt = std::chars_format::hex;
//		err = wtr.write("0x");
//		break;
//	default:
//		err = err::invalid_format;
//	}
//	if (!err.has_error()) {
//		if (specs.precision == NoPrecision) {
//			if (specs.type == no_type) {
//				err = wtr.write(arg);
//			} else {
//				err = wtr.write(arg, fmt);
//			}
//		} else {
//			err = wtr.write(arg, fmt, specs.precision);
//		}
//	}
//	return err;
// return err::invalid_format;
//}

inline constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const std::string_view arg) noexcept {
  if (specs.type != '?') {
    return write_padded<alignment::left>(wtr, specs, arg.size(), [&] {
      return wtr.write_str(arg);
    });
  }
  return write_padded<alignment::left>(wtr, specs, arg.size() + 2U, [&] {
    return wtr.write_str_escaped(arg);
  });
}

template <typename Arg>
  requires(std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const Arg arg) noexcept {
  // If a type other than None/c is specified, write out as integer instead of char.
  if (specs.type != no_type && specs.type != 'c' && specs.type != '?') {
    return write_arg(wtr, specs, static_cast<uint8_t>(arg));
  }
  if (specs.type != '?') {
    return write_padded<alignment::left>(wtr, specs, 1, [&] {
      return wtr.write_char(arg);
    });
  }
  return write_padded<alignment::left>(wtr, specs, 3, [&] {
    return wtr.write_char_escaped(arg);
  });
}

template <typename Arg>
  requires(std::is_same_v<Arg, void*> || std::is_same_v<Arg, std::nullptr_t>)
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, Arg arg) noexcept {
  specs.alternate_form = true;
  specs.type = 'x';
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): valid cast
  return write_arg(wtr, specs, reinterpret_cast<uintptr_t>(arg));
}

template <typename Arg>
  requires(std::is_same_v<Arg, bool>)
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, Arg arg) noexcept {
  // If a type other than None/s is specified, write out as 1/0 instead of true/false.
  if (specs.type != no_type && specs.type != 's') {
    return write_arg(wtr, specs, static_cast<uint8_t>(arg));
  }
  if (arg) {
    return write_padded<alignment::left>(wtr, specs, 4, [&] {
      return wtr.write_str("true");
    });
  }
  return write_padded<alignment::left>(wtr, specs, 5, [&] {
    return wtr.write_str("false");
  });
}

//
// Checks.
//

// specs is passed by reference instead as return type to reduce copying of big value (and code bloat)
inline constexpr result<void> validate_format_specs(reader<char>& rdr, format_specs& specs) noexcept {
  EMIO_TRY(char c, rdr.read_char());
  if (c == '}') {  // Format end.
    return success;
  }
  if (c == '{') {  // No dynamic spec support.
    return err::invalid_format;
  }

  bool width_required = false;
  {
    // Parse for alignment specifier.
    EMIO_TRY(const char c2, rdr.peek());
    if (c2 == '<' || c2 == '^' || c2 == '>') {
      if (c2 == '<') {
        specs.align = alignment::left;
      } else if (c2 == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      width_required = true;
      specs.fill = c;
      rdr.pop();
      EMIO_TRY(c, rdr.read_char());
    } else if (c == '<' || c == '^' || c == '>') {
      if (c == '<') {
        specs.align = alignment::left;
      } else if (c == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      width_required = true;
      EMIO_TRY(c, rdr.read_char());
    }
  }
  if (c == '+' || c == '-' || c == ' ') {  // Sign.
    specs.sign = c;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '0') {          // Zero flag.
    if (width_required) {  // Fill and zero flag doesn't make sense.
      return err::invalid_format;
    }
    specs.fill = '0';
    specs.align = alignment::right;
    specs.zero_flag = true;
    width_required = true;
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    EMIO_TRY(specs.width, rdr.parse_int<int>());
    EMIO_TRY(c, rdr.read_char());
  } else if (width_required) {  // Width was required.
    return err::invalid_format;
  }
  if (c == '.') {  // Precision.
    EMIO_TRY(specs.precision, rdr.parse_int<int>());
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isalpha(c) || c == '?') {  // Type.
    specs.type = c;
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '}') {  // Format end.
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> parse_format_specs(reader<char>& rdr, format_specs& specs) noexcept {
  char c = rdr.read_char().assume_value();
  if (c == '}') {  // Format end.
    return success;
  }

  {
    // Parse for alignment specifier.
    const char c2 = rdr.peek().assume_value();
    if (c2 == '<' || c2 == '^' || c2 == '>') {
      if (c2 == '<') {
        specs.align = alignment::left;
      } else if (c2 == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      specs.fill = c;
      rdr.pop();
      c = rdr.read_char().assume_value();
    } else if (c == '<' || c == '^' || c == '>') {
      if (c == '<') {
        specs.align = alignment::left;
      } else if (c == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      c = rdr.read_char().assume_value();
    }
  }
  if (c == '+' || c == '-' || c == ' ') {  // Sign.
    specs.sign = c;
    c = rdr.read_char().assume_value();
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    c = rdr.read_char().assume_value();
  }
  if (c == '0') {  // Zero flag.
    specs.fill = '0';
    specs.align = alignment::right;
    specs.zero_flag = true;
    c = rdr.read_char().assume_value();
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    specs.width = rdr.parse_int<int>().assume_value();
    c = rdr.read_char().assume_value();
  }
  if (c == '.') {  // Precision.
    specs.precision = rdr.parse_int<int>().assume_value();
    c = rdr.read_char().assume_value();
  }
  if (detail::isalpha(c) || c == '?') {  // Type.
    specs.type = c;
    rdr.pop();  // rdr.read_char() in validate_format_spec;
  }
  return success;
}

inline constexpr result<void> check_integral_specs(const format_specs& specs) noexcept {
  if (specs.precision != no_precision) {
    return err::invalid_format;
  }
  switch (specs.type) {
  case no_type:
  case 'b':
  case 'B':
  case 'c':
  case 'd':
  case 'o':
  case 'O':
  case 'x':
  case 'X':
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_unsigned_specs(const format_specs& specs) noexcept {
  if (specs.sign == no_sign) {
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_bool_specs(const format_specs& specs) noexcept {
  if (specs.type != no_type && specs.type != 's') {
    return check_integral_specs(specs);
  }
  if (specs.precision != no_precision) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_char_specs(const format_specs& specs) noexcept {
  if (specs.type != no_type && specs.type != 'c' && specs.type != '?') {
    return check_integral_specs(specs);
  }
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag || specs.precision != no_precision) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_pointer_specs(const format_specs& specs) noexcept {
  if (specs.type != no_type && specs.type != 'p') {
    return err::invalid_format;
  }
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag || specs.precision != no_precision) {
    return err::invalid_format;
  }
  return success;
}

inline constexpr result<void> check_floating_point_specs(const format_specs& specs) noexcept {
  switch (specs.type) {
  case no_type:
  case 'f':
  case 'e':
  case 'g':
  case 'a':
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> check_string_view_specs(const format_specs& specs) noexcept {
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag || specs.precision != no_precision ||
      (specs.type != no_type && specs.type != 's' && specs.type != '?')) {
    return err::invalid_format;
  }
  return success;
}

// Specifies if T has an enabled formatter specialization.
template <typename Arg>
inline constexpr bool has_formatter_v = std::is_constructible_v<formatter<Arg>>;

template <typename T>
concept has_validate_function_v = requires {
                                    {
                                      formatter<T>::validate(std::declval<reader<char>&>())
                                    } -> std::same_as<result<void>>;
                                  };

template <typename T>
concept has_any_validate_function_v =
    requires { &formatter<T>::validate; } || std::is_member_function_pointer_v<decltype(&formatter<T>::validate)> ||
    requires { std::declval<formatter<T>>().validate(std::declval<reader<char>&>()); };

template <typename Arg>
constexpr result<void> validate_for(reader<char>& format_is) noexcept {
  // Check if a formatter exist and a correct validate method is implemented. If not, use the parse method.
  if constexpr (has_formatter_v<Arg>) {
    if constexpr (has_validate_function_v<Arg>) {
      return formatter<Arg>::validate(format_is);
    } else {
      static_assert(!has_any_validate_function_v<Arg>,
                    "Formatter seems to have a validate property which doesn't fit the desired signature.");
      return formatter<Arg>{}.parse(format_is);
    }
  } else {
    static_assert(has_formatter_v<Arg>,
                  "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
    return err::invalid_format;
  }
}

template <typename T>
inline constexpr bool is_core_type_v =
    std::is_integral_v<T> || std::is_floating_point_v<T> || std::is_same_v<T, std::nullptr_t> ||
    std::is_same_v<T, void*> || std::is_constructible_v<std::string_view, T>;

template <input_validation FormatStringValidation, typename T>
concept formatter_parse_supports_format_string_validation =
    requires(T formatter) { formatter.template parse<FormatStringValidation>(std::declval<reader<char>>()); };

template <input_validation FormatStringValidation, typename T>
inline constexpr result<void> invoke_formatter_parse(T& formatter, reader<char>& format_is) noexcept {
  if constexpr (formatter_parse_supports_format_string_validation<FormatStringValidation, T>) {
    return formatter.template parse<FormatStringValidation>(format_is);
  } else {
    return formatter.parse(format_is);
  }
}

}  // namespace detail::format
}  // namespace emio
