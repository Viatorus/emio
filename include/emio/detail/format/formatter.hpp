//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
#include "../../writer.hpp"
#include "../misc.hpp"
#include "dragon.hpp"
#include "specs.hpp"

namespace emio {

template <typename>
class formatter;

namespace detail::format {

namespace alternate_form {

inline constexpr std::string_view bin_lower{"0b"};
inline constexpr std::string_view bin_upper{"0B"};
inline constexpr std::string_view octal{"0"};
inline constexpr std::string_view octal_lower{"0o"};
inline constexpr std::string_view octal_upper{"0O"};
inline constexpr std::string_view hex_lower{"0x"};
inline constexpr std::string_view hex_upper{"0X"};

}  // namespace alternate_form

//
// Write args.
//

inline constexpr result<void> write_padding_left(writer& out, format_specs& specs, size_t width) noexcept {
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
  return out.write_char_n(specs.fill, static_cast<size_t>(fill_width));
}

inline constexpr result<void> write_padding_right(writer& out, format_specs& specs) noexcept {
  if (specs.width == 0 || (specs.align != alignment::left && specs.align != alignment::center)) {
    return success;
  }
  return out.write_char_n(specs.fill, static_cast<size_t>(specs.width));
}

template <alignment DefaultAlign, typename Func>
constexpr result<void> write_padded(writer& out, format_specs& specs, size_t width, const Func& func) noexcept {
  if (specs.align == alignment::none) {
    specs.align = DefaultAlign;
  }
  EMIO_TRYV(write_padding_left(out, specs, width));
  EMIO_TRYV(func());
  return write_padding_right(out, specs);
}

inline constexpr result<std::pair<std::string_view, writer::write_int_options>> make_write_int_options(
    char spec_type) noexcept {
  using namespace alternate_form;

  std::string_view prefix;
  writer::write_int_options options{};

  switch (spec_type) {
  case no_type:
  case 'd':
    options.base = 10;
    break;
  case 'x':
    prefix = hex_lower;
    options.base = 16;
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

inline constexpr result<char> try_write_sign(writer& out, const format_specs& specs, bool is_negative) noexcept {
  char sign_to_write = no_sign;
  if (is_negative) {
    sign_to_write = '-';
  } else if (specs.sign == '+' || specs.sign == ' ') {
    sign_to_write = specs.sign;
  }
  if (sign_to_write != no_sign && specs.zero_flag) {
    EMIO_TRYV(out.write_char(sign_to_write));
    return no_sign;
  }
  return sign_to_write;
}

inline constexpr result<std::string_view> try_write_prefix(writer& out, const format_specs& specs,
                                                           std::string_view prefix) noexcept {
  const bool write_prefix = specs.alternate_form && !prefix.empty();
  if (write_prefix && specs.zero_flag) {
    EMIO_TRYV(out.write_str(prefix));
    return ""sv;
  }
  if (write_prefix) {
    return prefix;
  }
  return ""sv;
}

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer& out, format_specs& specs, const Arg& arg) noexcept {
  if (specs.type == 'c') {
    return write_padded<alignment::left>(out, specs, 1, [&]() noexcept {
      return out.write_char(static_cast<char>(arg));
    });
  }
  EMIO_TRY((auto [prefix, options]), make_write_int_options(specs.type));

  if (specs.type == 'o' && arg == 0) {
    prefix = ""sv;
  }

  const auto abs_number = detail::to_absolute(arg);
  const bool is_negative = detail::is_negative(arg);
  const size_t num_digits = detail::get_number_of_digits(abs_number, options.base);

  EMIO_TRY(const char sign_to_write, try_write_sign(out, specs, is_negative));
  EMIO_TRY(const std::string_view prefix_to_write, try_write_prefix(out, specs, prefix));

  size_t total_width = num_digits;
  if (specs.alternate_form) {
    total_width += prefix.size();
  }
  if (is_negative || specs.sign == ' ' || specs.sign == '+') {
    total_width += 1;
  }

  return write_padded<alignment::right>(out, specs, total_width, [&, &opt = options]() noexcept -> result<void> {
    const size_t area_size = num_digits + static_cast<size_t>(sign_to_write != no_sign) + prefix_to_write.size();
    EMIO_TRY(auto area, out.get_buffer().get_write_area_of(area_size));
    auto* it = area.data();
    if (sign_to_write != no_sign) {
      *it++ = sign_to_write;
    }
    if (!prefix_to_write.empty()) {
      it = copy_n(prefix_to_write.data(), prefix_to_write.size(), it);
    }
    write_number(abs_number, opt.base, opt.upper_case, it + detail::to_signed(num_digits));
    return success;
  });
}

inline constexpr result<void> write_non_finite(writer& out, bool upper_case, bool is_inf) noexcept {
  if (is_inf) {
    EMIO_TRYV(out.write_str(upper_case ? "INF"sv : "inf"sv));
  } else {
    EMIO_TRYV(out.write_str(upper_case ? "NAN"sv : "nan"sv));
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

inline constexpr fp_format_specs parse_fp_format_specs(const format_specs& specs) noexcept {
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
                                         char decimal_point) noexcept {
  out = copy_n(significand, integral_size, out);
  if (decimal_point == 0) {
    return out;
  }
  *out++ = decimal_point;
  return copy_n(significand + integral_size, significand_size - integral_size, out);
}

inline constexpr char* write_exponent(char* it, int exp) noexcept {
  if (exp < 0) {
    *it++ = '-';
    exp = -exp;
  } else {
    *it++ = '+';
  }
  int cnt = 2;
  if (exp >= 100) {
    write_decimal(to_unsigned(exp), it + 3);
    return it;
  } else if (exp < 10) {
    *it++ = '0';
    cnt -= 1;
  }
  write_decimal(to_unsigned(exp), it + cnt);
  return it;
}

inline constexpr result<void> write_decimal(writer& out, format_specs& specs, fp_format_specs& fp_specs,
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

  const auto use_exp_format = [=]() noexcept {
    if (fp_specs.format == fp_format::exp) {
      return true;
    }
    if (fp_specs.format != fp_format::general) {
      return false;
    }
    // Use the fixed notation if the exponent is in [exp_lower, exp_upper),
    // e.g. 0.0001 instead of 1e-04. Otherwise, use the exponent notation.
    constexpr int exp_lower = -4;
    constexpr int exp_upper = 16;
    return output_exp < exp_lower || output_exp >= (fp_specs.precision > 0 ? fp_specs.precision : exp_upper);
  };

  EMIO_TRY(const char sign_to_write, try_write_sign(out, specs, is_negative));

  int num_zeros = 0;
  char decimal_point = '.';
  size_t total_width = static_cast<uint32_t>(has_sign);
  size_t num_digits = to_unsigned(significand_size);

  if (use_exp_format()) {
    if (fp_specs.showpoint) {                             // Multiple significands or high precision.
      num_zeros = fp_specs.precision - significand_size;  // Trailing zeros after an zero only.
      if (num_zeros < 0) {
        num_zeros = 0;
      }
      num_digits += to_unsigned(num_zeros);
    } else if (significand_size == 1) {  // One significand.
      decimal_point = 0;
    }
    // The else part is general format with significand size less than the exponent.

    const int exp_digits = abs_output_exp >= 100 ? 3 : 2;
    num_digits += to_unsigned((decimal_point != 0 ? 1 : 0) + 2 /* sign + e */ + exp_digits);
    total_width += num_digits;

    return write_padded<alignment::right>(out, specs, total_width, [&]() noexcept -> result<void> {
      const size_t area_size = num_digits + static_cast<size_t>(sign_to_write != no_sign);
      EMIO_TRY(auto area, out.get_buffer().get_write_area_of(area_size));
      auto* it = area.data();
      if (sign_to_write != no_sign) {
        *it++ = sign_to_write;
      }

      it = write_significand(it, significand, significand_size, 1, decimal_point);
      it = fill_n(it, num_zeros, '0');
      *it++ = fp_specs.upper_case ? 'E' : 'e';
      write_exponent(it, output_exp);

      return success;
    });
  }

  int integral_size = 0;
  int num_zeros_2 = 0;

  if (output_exp < 0) {                                                   // Only fractional-part.
    num_digits += 2;                                                      // For zero + Decimal point.
    num_zeros = abs_output_exp - 1;                                       // Leading zeros after dot.
    if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#g}, 0.1) -> 0.100000 instead 0.1
      num_zeros_2 = fp_specs.precision - significand_size;
    }
  } else if ((output_exp + 1) >= significand_size) {  // Only integer-part (including zero).
    integral_size = significand_size;
    num_zeros = output_exp - significand_size + 1;  // Trailing zeros.
    if (fp_specs.showpoint) {                       // Significand is zero but fractional requested.
      if (specs.alternate_form && fp_specs.format == fp_format::general) {  // ({:#.4g}, 1) -> 1.000 instead of 1.
        num_zeros_2 = fp_specs.precision - significand_size - num_zeros;
      } else if (num_zeros == 0) {  // ({:f}, 0) or ({:.4f}, 1.23e-06) -> 0.000000 instead of 0
        num_zeros_2 = fp_specs.precision;
      }
      EMIO_Z_DEV_ASSERT(num_zeros >= 0);
      num_digits += 1;
    } else {  // Digit without zero
      decimal_point = 0;
    }
  } else {  // Both parts. Trailing zeros are part of significands.
    integral_size = output_exp + 1;
    num_digits += 1;                                                      // Decimal point.
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
  num_digits += static_cast<size_t>(num_zeros + num_zeros_2);
  total_width += num_digits;

  return write_padded<alignment::right>(out, specs, total_width, [&]() noexcept -> result<void> {
    const size_t area_size = num_digits + static_cast<size_t>(sign_to_write != no_sign);
    EMIO_TRY(auto area, out.get_buffer().get_write_area_of(area_size));
    auto* it = area.data();
    if (sign_to_write != no_sign) {
      *it++ = sign_to_write;
    }

    if (output_exp < 0) {
      *it++ = '0';
      if (decimal_point != 0) {
        *it++ = decimal_point;
        it = fill_n(it, num_zeros, '0');  // TODO: simplify fill_n/copy/copy/n + it
        it = copy_n(significand, significand_size, it);
        fill_n(it, num_zeros_2, '0');
      }
    } else if ((output_exp + 1) >= significand_size) {
      it = copy_n(significand, integral_size, it);
      if (num_zeros != 0) {
        it = fill_n(it, num_zeros, '0');
      }
      if (decimal_point != 0) {
        *it++ = '.';
        if (num_zeros_2 != 0) {
          fill_n(it, num_zeros_2, '0');
        }
      }
    } else {
      it = write_significand(it, significand, significand_size, integral_size, decimal_point);
      if (num_zeros != 0) {
        fill_n(it, num_zeros, '0');
      }
    }

    return success;
  });
}

inline constexpr std::array<char, 1> zero_digit{'0'};

inline constexpr format_fp_result_t format_decimal(buffer& buffer, const fp_format_specs& fp_specs,
                                                   const decode_result_t& decoded) noexcept {
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
    return format_exact(decoded.finite, buffer, format_exact_mode::significand_digits, fp_specs.precision);
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

inline constexpr result<void> format_and_write_decimal(writer& out, format_specs& specs,
                                                       const decode_result_t& decoded) noexcept {
  fp_format_specs fp_specs = parse_fp_format_specs(specs);

  if (decoded.category == category::infinity || decoded.category == category::nan) {
    if (specs.zero_flag) {  // Words aren't prefixed with zeros.
      specs.fill = ' ';
      specs.zero_flag = false;
    }
    EMIO_TRY(const char sign_to_write, try_write_sign(out, specs, decoded.negative));

    const size_t total_length = 3 + static_cast<uint32_t>(sign_to_write != no_sign);
    return write_padded<alignment::left>(out, specs, total_length, [&]() noexcept -> result<void> {
      if (sign_to_write != no_sign) {
        EMIO_TRYV(out.write_char(sign_to_write));
      }
      return write_non_finite(out, fp_specs.upper_case, decoded.category == category::infinity);
    });
  }

  emio::memory_buffer buf;
  const format_fp_result_t res = format_decimal(buf, fp_specs, decoded);
  return write_decimal(out, specs, fp_specs, decoded.negative, res);
}

template <typename Arg>
  requires(std::is_floating_point_v<Arg> && sizeof(Arg) <= sizeof(double))
constexpr result<void> write_arg(writer& out, format_specs& specs, const Arg& arg) noexcept {
  return format_and_write_decimal(out, specs, decode(arg));
}

inline constexpr result<void> write_arg(writer& out, format_specs& specs, std::string_view arg) noexcept {
  if (specs.type != '?') {
    if (specs.precision >= 0) {
      arg = unchecked_substr(arg, 0, static_cast<size_t>(specs.precision));
    }
    return write_padded<alignment::left>(out, specs, arg.size(), [&]() noexcept {
      return out.write_str(arg);
    });
  }
  const size_t escaped_size = detail::count_size_when_escaped(arg);
  return write_padded<alignment::left>(out, specs, escaped_size + 2U /* quotes */, [&]() noexcept {
    return detail::write_str_escaped(out.get_buffer(), arg, escaped_size, '"');
  });
}

template <typename Arg>
  requires(std::is_same_v<Arg, char>)
constexpr result<void> write_arg(writer& out, format_specs& specs, const Arg arg) noexcept {
  // If a type other than None/c is specified, write out as integer instead of char.
  if (specs.type != no_type && specs.type != 'c' && specs.type != '?') {
    return write_arg(out, specs, static_cast<uint8_t>(arg));
  }
  if (specs.type != '?') {
    return write_padded<alignment::left>(out, specs, 1, [&]() noexcept {
      return out.write_char(arg);
    });
  }
  return write_padded<alignment::left>(out, specs, 3, [&]() noexcept {
    return out.write_char_escaped(arg);
  });
}

template <typename T>
inline constexpr bool is_void_pointer_v =
    std::is_pointer_v<T> && std::is_same_v<std::remove_cv_t<std::remove_pointer_t<T>>, void>;

template <typename Arg>
  requires(is_void_pointer_v<Arg> || std::is_null_pointer_v<Arg>)
constexpr result<void> write_arg(writer& out, format_specs& specs, Arg arg) noexcept {
  specs.alternate_form = true;
  specs.type = 'x';
  if constexpr (std::is_null_pointer_v<Arg>) {
    return write_arg(out, specs, uintptr_t{0});
  } else {
    return write_arg(out, specs, std::bit_cast<uintptr_t>(arg));
  }
}

template <typename Arg>
  requires(std::is_same_v<Arg, bool>)
constexpr result<void> write_arg(writer& out, format_specs& specs, Arg arg) noexcept {
  // If a type other than None/s is specified, write out as 1/0 instead of true/false.
  if (specs.type != no_type && specs.type != 's') {
    return write_arg(out, specs, static_cast<uint8_t>(arg));
  }
  if (arg) {
    return write_padded<alignment::left>(out, specs, 4, [&]() noexcept {
      return out.write_str("true"sv);
    });
  }
  return write_padded<alignment::left>(out, specs, 5, [&]() noexcept {
    return out.write_str("false"sv);
  });
}

//
// Checks.
//

// specs is passed by reference instead as return type to reduce copying of big value (and code bloat)
inline constexpr result<void> validate_format_specs(reader& format_rdr, format_specs& specs) noexcept {
  EMIO_TRY(char c, format_rdr.read_char());
  if (c == '}') {  // Format end.
    return success;
  }
  if (c == '{') {  // No dynamic spec support.
    return err::invalid_format;
  }

  {
    // Parse for alignment specifier.
    EMIO_TRY(const char c2, format_rdr.peek());
    if (c2 == '<' || c2 == '^' || c2 == '>') {
      if (c2 == '<') {
        specs.align = alignment::left;
      } else if (c2 == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      specs.fill = c;
      format_rdr.pop();
      EMIO_TRY(c, format_rdr.read_char());
    } else if (c == '<' || c == '^' || c == '>') {
      if (c == '<') {
        specs.align = alignment::left;
      } else if (c == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      EMIO_TRY(c, format_rdr.read_char());
    }
  }
  if (c == '+' || c == '-' || c == ' ') {  // Sign.
    specs.sign = c;
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (c == '0') {                          // Zero flag.
    if (specs.align == alignment::none) {  // If fill/align is used, the zero flag is ignored.
      specs.fill = '0';
      specs.align = alignment::right;
      specs.zero_flag = true;
    }
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (detail::isdigit(c)) {  // Width.
    format_rdr.unpop();
    EMIO_TRY(const uint32_t width, format_rdr.parse_int<uint32_t>());
    if (width > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.width = static_cast<int32_t>(width);
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (c == '.') {  // Precision.
    if (const result<char> next = format_rdr.peek();
        next && !isdigit(next.assume_value())) {  // Not followed by a digit.
      return err::invalid_format;
    }
    EMIO_TRY(const uint32_t precision, format_rdr.parse_int<uint32_t>());
    if (precision > (static_cast<uint32_t>(std::numeric_limits<int32_t>::max()))) {
      return err::invalid_format;
    }
    specs.precision = static_cast<int32_t>(precision);
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (detail::isalpha(c) || c == '?') {  // Type.
    specs.type = c;
    EMIO_TRY(c, format_rdr.read_char());
  }
  if (c == '}') {  // Format end.
    return success;
  }
  return err::invalid_format;
}

inline constexpr result<void> parse_format_specs(reader& format_rdr, format_specs& specs) noexcept {
  char c = format_rdr.read_char().assume_value();
  if (c == '}') {  // Format end.
    return success;
  }

  {
    // Parse for alignment specifier.
    const char c2 = format_rdr.peek().assume_value();
    if (c2 == '<' || c2 == '^' || c2 == '>') {
      if (c2 == '<') {
        specs.align = alignment::left;
      } else if (c2 == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      specs.fill = c;
      format_rdr.pop();
      c = format_rdr.read_char().assume_value();
    } else if (c == '<' || c == '^' || c == '>') {
      if (c == '<') {
        specs.align = alignment::left;
      } else if (c == '^') {
        specs.align = alignment::center;
      } else {
        specs.align = alignment::right;
      }
      c = format_rdr.read_char().assume_value();
    }
  }
  if (c == '+' || c == '-' || c == ' ') {  // Sign.
    specs.sign = c;
    c = format_rdr.read_char().assume_value();
  }
  if (c == '#') {  // Alternate form.
    specs.alternate_form = true;
    c = format_rdr.read_char().assume_value();
  }
  if (c == '0') {                          // Zero flag.
    if (specs.align == alignment::none) {  // Ignoreable.
      specs.fill = '0';
      specs.align = alignment::right;
      specs.zero_flag = true;
    }
    c = format_rdr.read_char().assume_value();
  }
  if (detail::isdigit(c)) {  // Width.
    format_rdr.unpop();
    specs.width = static_cast<int32_t>(format_rdr.parse_int<uint32_t>().assume_value());
    c = format_rdr.read_char().assume_value();
  }
  if (c == '.') {  // Precision.
    specs.precision = static_cast<int32_t>(format_rdr.parse_int<uint32_t>().assume_value());
    c = format_rdr.read_char().assume_value();
  }
  if (detail::isalpha(c) || c == '?') {  // Type.
    specs.type = c;
    format_rdr.pop();  // format_rdr.read_char() in validate_format_specs;
  }
  return success;
}

inline constexpr result<void> check_integral_specs(const format_specs& specs) noexcept {
  if (specs.precision != no_precision) {
    return err::invalid_format;
  }
  switch (specs.type) {
  case no_type:
  case 'd':
  case 'x':
  case 'X':
  case 'b':
  case 'B':
  case 'c':
  case 'o':
  case 'O':
    return success;
  default:
    return err::invalid_format;
  }
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
    //  case 'a': Not supported yet.
    //  case 'A':
    return success;
  default:
    return err::invalid_format;
  }
}

inline constexpr result<void> check_string_specs(const format_specs& specs) noexcept {
  if (specs.alternate_form || specs.sign != no_sign || specs.zero_flag ||
      (specs.precision != no_precision && specs.type == '?') ||
      (specs.type != no_type && specs.type != 's' && specs.type != '?')) {
    return err::invalid_format;
  }
  return success;
}

//
// Type traits.
//

// Specifies if T has an enabled formatter specialization.
template <typename Arg>
inline constexpr bool has_formatter_v = std::is_constructible_v<formatter<Arg>>;

template <typename T>
concept has_validate_function_v = requires {
  { formatter<T>::validate(std::declval<reader&>()) } -> std::same_as<result<void>>;
};

template <typename T>
concept has_static_validate_function_v = requires { &formatter<T>::validate; };

template <typename T>
concept has_member_validate_function_v = requires { std::declval<formatter<T>>().validate(std::declval<reader&>()); };

template <typename T>
concept has_any_validate_function_v =
    has_static_validate_function_v<T> || std::is_member_function_pointer_v<decltype(&formatter<T>::validate)> ||
    has_member_validate_function_v<T>;

template <typename Arg>
constexpr result<void> validate_trait(reader& format_rdr) {
  // Check if a formatter exist and a correct validate method is implemented. If not, use the parse method.
  if constexpr (has_formatter_v<Arg>) {
    if constexpr (has_validate_function_v<Arg>) {
      return formatter<Arg>::validate(format_rdr);
    } else {
      static_assert(!has_any_validate_function_v<Arg>,
                    "Formatter seems to have a validate property which doesn't fit the desired signature.");
      return formatter<Arg>{}.parse(format_rdr);
    }
  } else {
    static_assert(has_formatter_v<Arg>,
                  "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
    return err::invalid_format;
  }
}

template <typename T>
inline constexpr bool is_core_type_v =
    std::is_same_v<T, bool> || std::is_same_v<T, char> || std::is_same_v<T, int32_t> || std::is_same_v<T, uint32_t> ||
    std::is_same_v<T, int64_t> || std::is_same_v<T, uint64_t> || std::is_same_v<T, double> ||
    std::is_null_pointer_v<T> || is_void_pointer_v<T> || std::is_same_v<T, std::string_view>;

template <typename T>
concept has_format_as = requires(T arg) { format_as(arg); };

template <typename T>
using format_as_return_t = decltype(format_as(std::declval<T>()));

// To reduce code bloat, similar types are unified to a general one.
template <typename T>
struct unified_type;

template <typename T>
struct unified_type {
  using type = const T&;
};

template <typename T>
  requires(!std::is_integral_v<T> && !std::is_null_pointer_v<T> && std::is_constructible_v<std::string_view, T>)
struct unified_type<T> {
  using type = std::string_view;
};

template <typename T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
struct unified_type<T> {
  using type = std::conditional_t<num_bits<T>() <= 32, int32_t, int64_t>;
};

template <typename T>
  requires(std::is_floating_point_v<T> && sizeof(T) <= sizeof(double))
struct unified_type<T> {
  using type = double;
};

template <typename T>
  requires(std::is_same_v<T, char> || std::is_same_v<T, bool> || is_void_pointer_v<T> || std::is_null_pointer_v<T>)
struct unified_type<T> {
  using type = T;
};

template <typename T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
struct unified_type<T> {
  using type = std::conditional_t<num_bits<T>() <= 32, uint32_t, uint64_t>;
};

template <typename T>
using unified_type_t = typename unified_type<T>::type;

}  // namespace detail::format
}  // namespace emio
