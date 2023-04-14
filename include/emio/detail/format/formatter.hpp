//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../writer.hpp"
#include "../parser.hpp"
#include "dragon.hpp"
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

inline constexpr result<void> write_sign(writer<char>& wtr, const format_specs& specs, bool negative) {
  if (negative) {
    EMIO_TRYV(wtr.write_char('-'));
  } else if (specs.sign == '+' || specs.sign == ' ') {
    EMIO_TRYV(wtr.write_char(specs.sign));
  }
  return success;
}

inline constexpr result<void> write_prefix(writer<char>& wtr, const format_specs& specs, std::string_view prefix) {
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
  const bool is_negative = detail::is_negative(arg);
  const size_t number_of_digits = detail::get_number_of_digits(abs_number, options.base);

  size_t total_length = number_of_digits;
  if (specs.alternate_form) {
    total_length += prefix.size();
  }
  if (is_negative || specs.sign == ' ' || specs.sign == '+') {
    total_length += 1;
  }

  if (specs.zero_flag) {
    EMIO_TRYV(write_sign(wtr, specs, is_negative));
    EMIO_TRYV(write_prefix(wtr, specs, prefix));
  }

  return write_padded<alignment::right>(
      wtr, specs, total_length, [&, &prefix = prefix, &options = options]() -> result<void> {
        if (!specs.zero_flag) {
          EMIO_TRYV(write_sign(wtr, specs, is_negative));
          EMIO_TRYV(write_prefix(wtr, specs, prefix));
        }

        auto& buf = wtr.get_buffer();
        EMIO_TRY(auto area, buf.get_write_area_of(number_of_digits));
        write_number(abs_number, options.base, options.upper_case, area.begin() + detail::to_signed(number_of_digits));
        return success;
      });
}

inline constexpr result<void> write_non_finite(writer<char>& wtr, bool upper_case, bool is_inf) {
  if (is_inf) {
    EMIO_TRYV(wtr.write_str(upper_case ? "INF" : "inf"));
  } else {
    EMIO_TRYV(wtr.write_str(upper_case ? "NAN" : "nan"));
  }
  return success;
}

// A floating-point presentation format.
enum class fp_format : uint8_t {
  general,  // General: exponent notation or fixed point based on magnitude.
  exp,      // Exponent notation with the default precision of 6, e.g. 1.2e-3.
  fixed,    // Fixed point with the default precision of 6, e.g. 0.0012.
  hex
};

struct fp_format_specs {
  int16_t precision;
  fp_format format;
  bool upper_case;
  bool showpoint;
};

inline constexpr fp_format_specs parse_fp_format_specs(const format_specs& specs) {
  constexpr int16_t default_precision = 6;

  // This spec is typically for general format.
  fp_format_specs fp_specs{
      .precision =
          specs.precision >= 0 || specs.type == no_type ? static_cast<int16_t>(specs.precision) : default_precision,
      .format = fp_format::general,
      .upper_case = specs.type == 'E' || specs.type == 'F' || specs.type == 'G',
      .showpoint = specs.alternate_form,
  };

  if (specs.type == 'e' || specs.type == 'E') {
    fp_specs.format = fp_format::exp;
    fp_specs.precision += 1;
    fp_specs.showpoint |= specs.precision != 0;
  } else if (specs.type == 'f' || specs.type == 'F') {
    fp_specs.format = fp_format::fixed;
    fp_specs.showpoint |= specs.precision != 0;
  } else if (specs.type == 'a' || specs.type == 'A') {
    fp_specs.format = fp_format::hex;
  }
  if (fp_specs.format != fp_format::fixed && fp_specs.precision == 0) {
    fp_specs.precision = 1;  // Calculate at least on significand.
  }
  return fp_specs;
}

inline constexpr char* write_significand(char* out, const char* significand, int significand_size, int integral_size,
                                         char decimal_point) {
  std::copy(significand, significand + integral_size, out);
  out += integral_size;
  if (!decimal_point) {
    return out;
  }
  *out++ = decimal_point;
  std::copy(significand + integral_size, significand + significand_size, out);
  return out + significand_size - integral_size;
}

inline constexpr char* write_exponent(char* it, int exp) {
  if (exp < 0) {
    *it++ = '-';
    exp = -exp;
  } else {
    *it++ = '+';
  }
  int cnt = 2;
  if (exp >= 100) {
    write_number(to_unsigned(exp), 10, false, it + 3);
    return it;
  } else if (exp < 10) {
    *it++ = '0';
    cnt -= 1;
  }
  write_number(to_unsigned(exp), 10, false, it + cnt);
  return it;
}

inline constexpr result<void> write_decimal(writer<char>& wtr, format_specs& specs, fp_format_specs& fp_specs,
                                            bool is_negative, const format_fp_result_t& f) noexcept {
  const char* significand = f.digits.data();
  int significand_size = static_cast<int>(f.digits.size());
  const int output_exp = f.exp - 1;  // 0.1234 x 10^exp => 1.234 x 10^(exp-1)
  const int abs_output_exp = static_cast<uint16_t>(output_exp >= 0 ? output_exp : -output_exp);
  const bool has_sign = is_negative || specs.sign == ' ' || specs.sign == '+';

  if (fp_specs.format == fp_format::general && significand_size > 1) {
    // Remove trailing zeros.
    auto it = std::find_if(f.digits.rbegin(), f.digits.rend(), [](char c) {
      return c != '0';
    });
    significand_size -= static_cast<int>(it - f.digits.rbegin());
  }

  const auto use_exp_format = [=]() {
    if (fp_specs.format == fp_format::exp) return true;
    if (fp_specs.format != fp_format::general) return false;
    // Use the fixed notation if the exponent is in [exp_lower, exp_upper),
    // e.g. 0.0001 instead of 1e-04. Otherwise, use the exponent notation.
    const int exp_lower = -4, exp_upper = 16;
    return output_exp < exp_lower || output_exp >= (fp_specs.precision > 0 ? fp_specs.precision : exp_upper);
  };

  if (specs.zero_flag) {
    EMIO_TRYV(write_sign(wtr, specs, is_negative));
  }

  int num_zeros = 0;
  char decimal_point = '.';
  size_t total_length = to_unsigned(significand_size);

  if (use_exp_format()) {
    if (fp_specs.showpoint) {                             // Multiple significands or high precision.
      num_zeros = fp_specs.precision - significand_size;  // Trailing zeros after an zero only.
      if (num_zeros < 0) {
        num_zeros = 0;
      }
      total_length += to_unsigned(num_zeros);
    } else if (significand_size == 1) {  // One significant.
      decimal_point = char{};
    }
    // The else part is general format with significand size less than the exponent.

    const int exp_digits = abs_output_exp >= 100 ? 3 : 2;
    total_length += to_unsigned((decimal_point ? 1 : 0) + 2 /* sign + e */ + exp_digits);

    return write_padded<alignment::right>(wtr, specs, total_length + has_sign, [&]() -> result<void> {
      if (!specs.zero_flag) {
        EMIO_TRYV(write_sign(wtr, specs, is_negative));
      }
      EMIO_TRY(auto area, wtr.get_buffer().get_write_area_of(total_length));
      auto it = area.data();

      it = write_significand(it, significand, significand_size, 1, decimal_point);
      std::fill_n(it, num_zeros, '0');
      it += num_zeros;
      *it++ = fp_specs.upper_case ? 'E' : 'e';
      write_exponent(it, output_exp);

      return success;
    });
  }

  int integral_size = 0;
  int num_zeros_2 = 0;

  if (output_exp < 0) {                                                   // Only fractional-part.
    total_length += 2;                                                    // For zero + Decimal point.
    num_zeros = abs_output_exp - 1;                                       // Leading zeros after dot.
    if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#g}, 0.1) -> 0.100000 instead 0.1
      num_zeros_2 = fp_specs.precision - significand_size;
    }
  } else if ((output_exp + 1) >= significand_size) {  // Only integer-part (including zero).
    integral_size = significand_size;
    num_zeros = output_exp - significand_size + 1;  // Trailing zeros.
    if (fp_specs.showpoint) {                       // Significant is zero but fractional requested.
      if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#.4g}, 1) -> 1.000 instead of 1.
        num_zeros_2 = fp_specs.precision - significand_size - num_zeros;
      } else if (num_zeros == 0) {  // ({:f}, 0) or ({:.4f}, 1.23e-06) -> 0.000000 instead of 0
        num_zeros_2 = fp_specs.precision;
      }
      EMIO_Z_DEV_ASSERT(num_zeros >= 0);
      total_length += 1;
    } else {  // Digit without zero
      decimal_point = 0;
    }
  } else {  // Both parts. Trailing zeros are part of significands.
    integral_size = output_exp + 1;
    total_length += 1;                                                    // Decimal point.
    if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#g}, 1.2) -> 1.20000 instead 1.2
      num_zeros = fp_specs.precision - significand_size;
    }
    if (fp_specs.format == fp_format::fixed && significand_size > integral_size &&
        significand_size - integral_size < fp_specs.precision) {  // ({:.4}, 0.99999) -> 1.0000 instead of 1.00
      num_zeros = fp_specs.precision - (significand_size - integral_size);
    }
  }
  if (num_zeros < 0) {
    num_zeros = 0;
  }
  if (num_zeros_2 < 0) {
    num_zeros_2 = 0;
  }
  total_length += static_cast<size_t>(num_zeros + num_zeros_2);

  return write_padded<alignment::right>(wtr, specs, total_length + has_sign, [&]() -> result<void> {
    if (!specs.zero_flag) {
      EMIO_TRYV(write_sign(wtr, specs, is_negative));
    }

    EMIO_TRY(auto area, wtr.get_buffer().get_write_area_of(total_length));
    auto it = area.data();

    if (output_exp < 0) {
      *it++ = '0';
      if (decimal_point) {
        *it++ = decimal_point;
        std::fill_n(it, num_zeros, '0');
        it += num_zeros;
        std::copy_n(significand, significand_size, it);
        it += significand_size;
        std::fill_n(it, num_zeros_2, '0');
      }
    } else if ((output_exp + 1) >= significand_size) {
      std::copy(significand, significand + integral_size, it);
      it += significand_size;
      if (num_zeros) {
        std::fill_n(it, num_zeros, '0');
        it += num_zeros;
      }
      if (decimal_point) {
        *it++ = '.';
        if (num_zeros_2) {
          std::fill_n(it, num_zeros_2, '0');
        }
      }
    } else {
      it = write_significand(it, significand, significand_size, integral_size, decimal_point);
      if (num_zeros) {
        std::fill_n(it, num_zeros, '0');
        it += num_zeros;
      }
    }

    return success;
  });
}

inline constexpr std::array<char, 1> zero_digit{'0'};

inline constexpr format_fp_result_t format_decimal(buffer<char>& buffer, const fp_format_specs& fp_specs,
                                                   const decode_result_t& decoded) {
  if (decoded.category == category::zero) {
    return format_fp_result_t{zero_digit, 1};
  }
  switch (fp_specs.format) {
  case fp_format::general:
    if (fp_specs.precision == no_precision) {
      return format_shortest(decoded.finite, buffer);
    }
    [[fallthrough]];
  case fp_format::exp:
    return format_exact(decoded.finite, buffer, format_exact_mode::significant_digits, fp_specs.precision);
  case fp_format::fixed: {
    auto res = format_exact(decoded.finite, buffer, format_exact_mode::decimal_point, fp_specs.precision);
    if (res.digits.empty()) {
      return format_fp_result_t{zero_digit, 1};
    }
    return res;
  }
  case fp_format::hex:
    std::terminate();
  }
  EMIO_Z_INTERNAL_UNREACHABLE;
}

inline constexpr result<void> write_decimal(writer<char>& wtr, format_specs& specs,
                                            const decode_result_t& decoded) noexcept {
  fp_format_specs fp_specs = parse_fp_format_specs(specs);

  if (decoded.category == category::infinity || decoded.category == category::nan) {
    size_t total_length = 3;
    if (decoded.negative || specs.sign == ' ' || specs.sign == '+') {
      total_length += 1;
    }
    if (specs.zero_flag) {  // Words aren't prefixed with zeros.
      specs.fill = ' ';
    }
    return write_padded<alignment::left>(wtr, specs, total_length, [&]() -> result<void> {
      EMIO_TRYV(write_sign(wtr, specs, decoded.negative));
      return write_non_finite(wtr, fp_specs.upper_case, decoded.category == category::infinity);
    });
  }

  emio::string_buffer buf;
  format_fp_result_t res = format_decimal(buf, fp_specs, decoded);
  return write_decimal(wtr, specs, fp_specs, decoded.negative, res);
}

template <typename Arg>
  requires(std::is_floating_point_v<Arg> && sizeof(Arg) <= sizeof(double))
constexpr result<void> write_arg(writer<char>& wtr, format_specs& specs, const Arg& arg) noexcept {
  return write_decimal(wtr, specs, decode(arg));
}

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

  bool fill_aligned = false;
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
      fill_aligned = true;
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
      fill_aligned = true;
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
  if (c == '0') {         // Zero flag.
    if (!fill_aligned) {  // If fill/align is used, the zero flag is ignored.
      specs.fill = '0';
      specs.align = alignment::right;
      specs.zero_flag = true;
      fill_aligned = true;
    }
    EMIO_TRY(c, rdr.read_char());
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    EMIO_TRY(uint32_t width, rdr.parse_int<uint32_t>());
    if (width > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.width = static_cast<int32_t>(width);
    EMIO_TRY(c, rdr.read_char());
  }
  if (c == '.') {  // Precision.
    EMIO_TRY(uint32_t precision, rdr.parse_int<uint32_t>());
    if (precision > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.precision = static_cast<int32_t>(precision);
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

  bool fill_aligned = false;
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
      fill_aligned = true;
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
      fill_aligned = true;
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
  if (c == '0') {         // Zero flag.
    if (!fill_aligned) {  // Ignoreable.
      specs.fill = '0';
      specs.align = alignment::right;
      specs.zero_flag = true;
    }
    c = rdr.read_char().assume_value();
  }
  if (detail::isdigit(c)) {  // Width.
    rdr.unpop();
    specs.width = static_cast<int32_t>(rdr.parse_int<uint32_t>().assume_value());
    c = rdr.read_char().assume_value();
  }
  if (c == '.') {  // Precision.
    specs.precision = static_cast<int32_t>(rdr.parse_int<uint32_t>().assume_value());
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
  if (specs.precision > 1100) {
    return err::invalid_format;
  }

  switch (specs.type) {
  case no_type:
  case 'f':
  case 'F':
  case 'e':
  case 'E':
  case 'g':
  case 'G':
  case 'a':
  case 'A':
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
