//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstdint>
#include <limits>
#include <optional>
#include <string_view>

#include "../reader.hpp"
#include "args.hpp"

namespace emio::detail {

/**
 * Flag to enable/disable input validation if already done
 */
enum class input_validation { enabled, disabled };

template <typename T>
struct always_false : std::false_type {};

template <typename T>
inline constexpr bool always_false_v = always_false<T>::value;

namespace alternate_form {

inline constexpr std::string_view bin_lower{"0b"};
inline constexpr std::string_view bin_upper{"0B"};
inline constexpr std::string_view octal{"0"};
inline constexpr std::string_view octal_lower{"0o"};
inline constexpr std::string_view octal_upper{"0O"};
inline constexpr std::string_view hex_lower{"0x"};
inline constexpr std::string_view hex_upper{"0X"};

}  // namespace alternate_form

inline constexpr uint8_t no_more_args = std::numeric_limits<uint8_t>::max();

template <input_validation>
class parser_base {
 public:
  constexpr explicit parser_base(reader& format_rdr) noexcept : format_rdr_{format_rdr} {}

  parser_base(const parser_base&) = delete;
  parser_base(parser_base&&) = delete;
  parser_base& operator=(const parser_base&) = delete;
  parser_base& operator=(parser_base&&) = delete;

  virtual constexpr ~parser_base() = default;

  constexpr result<void> parse(uint8_t& arg_nbr) noexcept {
    while (true) {
      result<char> res = format_rdr_.read_char();
      if (res == err::eof) {
        return success;
      }
      if (!res) {
        return res;
      }
      char c = res.assume_value();
      if (c == '{') {
        EMIO_TRY(c, format_rdr_.peek());  // If failed: Incorrect escaped {.
        if (c != '{') {
          return parse_replacement_field(arg_nbr);
        }
        format_rdr_.pop();
      } else if (c == '}') {
        EMIO_TRY(c, format_rdr_.peek());
        if (c != '}') {
          // Not escaped }.
          return err::invalid_format;
        }
        format_rdr_.pop();
      }
      EMIO_TRYV(process(c));
    }
  }

 protected:
  virtual constexpr result<void> process(char c) noexcept = 0;

  reader& format_rdr_;

 private:
  constexpr result<void> parse_replacement_field(uint8_t& arg_nbr) noexcept {
    EMIO_TRYV(parse_field_name(arg_nbr));

    EMIO_TRY(const char c, format_rdr_.peek());
    if (c == '}') {
      return success;
    }
    if (c == ':') {  // Format specs.
      // Format specs are parsed with known argument type later.
      format_rdr_.pop();
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> parse_field_name(uint8_t& arg_nbr) noexcept {
    EMIO_TRY(const char c, format_rdr_.peek());
    if (detail::isdigit(c)) {               // Positional argument.
      if (use_positional_args_ == false) {  // If first argument was positional -> failure.
        return err::invalid_format;
      }
      EMIO_TRY(arg_nbr, format_rdr_.template parse_int<uint8_t>());
      use_positional_args_ = true;
    } else {
      if (use_positional_args_ == true) {
        return err::invalid_format;
      }
      use_positional_args_ = false;
      // None positional argument. Increase arg_nbr after each format specifier.
      arg_nbr = increment_arg_number_++;
    }
    return success;
  }

  std::optional<bool> use_positional_args_{};
  uint8_t increment_arg_number_{};
};

template <>
class parser_base<input_validation::disabled> {
 public:
  constexpr explicit parser_base(reader& format_rdr) noexcept : format_rdr_{format_rdr} {}

  parser_base(const parser_base& other) = delete;
  parser_base(parser_base&& other) = delete;
  parser_base& operator=(const parser_base& other) = delete;
  parser_base& operator=(parser_base&& other) = delete;

  virtual constexpr ~parser_base() = default;

  constexpr result<void> parse(uint8_t& arg_nbr) noexcept {
    while (true) {
      result<char> res = format_rdr_.read_char();
      if (res == err::eof) {
        return success;
      }
      char c = res.assume_value();
      if (c == '{') {
        c = format_rdr_.peek().assume_value();
        if (c != '{') {
          return parse_replacement_field(arg_nbr);
        }
        format_rdr_.pop();
      } else if (c == '}') {
        format_rdr_.pop();
      }
      EMIO_TRYV(process(c));
    }
  }

 protected:
  virtual constexpr result<void> process(char c) noexcept = 0;

  reader& format_rdr_;

 private:
  constexpr result<void> parse_replacement_field(uint8_t& arg_nbr) noexcept {
    parse_field_name(arg_nbr);
    const char c = format_rdr_.peek().assume_value();
    if (c == '}') {
      return success;
    }
    format_rdr_.pop();
    return success;
  }

  constexpr void parse_field_name(uint8_t& arg_nbr) noexcept {
    const char c = format_rdr_.peek().assume_value();
    if (detail::isdigit(c)) {  // Positional argument.
      arg_nbr = format_rdr_.template parse_int<uint8_t>().assume_value();
      use_positional_args_ = true;
    } else {
      use_positional_args_ = false;
      // None positional argument. Increase arg_nbr after each format specifier.
      arg_nbr = increment_arg_number_++;
    }
  }

  std::optional<bool> use_positional_args_{};
  uint8_t increment_arg_number_{};
};

template <typename T>
int is_arg_span2(const args_span<T>& t);

bool is_arg_span2(...);

template <typename T>
constexpr bool is_args_span = sizeof(is_arg_span2(std::declval<T>())) == sizeof(int);

template <typename CRTP, input_validation Validation>
class parser : public parser_base<Validation> {
 public:
  using parser_base<Validation>::parser_base;

  parser(const parser&) = delete;
  parser(parser&&) = delete;
  parser& operator=(const parser&) = delete;
  parser& operator=(parser&&) = delete;
  constexpr ~parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  template <typename T>
  result<void> apply(uint8_t arg_nbr, const args_span<T>& args) noexcept {
    return static_cast<CRTP*>(this)->process_arg(args.get_args()[arg_nbr]);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> apply(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
    requires(!is_args_span<Arg>)
  constexpr result<void> apply(uint8_t arg_pos, Arg& arg, Args&... args) noexcept {
    if (arg_pos == 0) {
      return static_cast<CRTP*>(this)->process_arg(arg);
    }
    return apply(arg_pos - 1, args...);
  }
};

// Explicit out-of-class definition because of GCC bug: <destructor> used before its definition.
template <typename CRTP, input_validation Validation>
constexpr parser<CRTP, Validation>::~parser() noexcept = default;

template <typename Parser, typename... Args>
constexpr bool validate(std::string_view str, const size_t arg_cnt, const Args&... args) {
  reader format_rdr{str};
  Parser parser{format_rdr};
  bitset<128> matched{};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = parser.parse(arg_nbr); !res) {
      return false;
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (arg_cnt <= arg_nbr) {
      return false;
    }
    matched.set(arg_nbr);
    auto res = parser.apply(arg_nbr, args...);
    if (!res) {
      return false;
    }
  }
  return matched.all_first(arg_cnt);
}

template <typename Parser, typename T, typename... Args>
constexpr result<void> parse(std::string_view str, T& input, Args&&... args) {
  reader format_rdr{str};
  Parser parser{input, format_rdr};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = parser.parse(arg_nbr); !res) {
      return res.assume_error();
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (auto res = parser.apply(arg_nbr, std::forward<Args>(args)...); !res) {
      return res.assume_error();
    }
  }
  return success;
}

}  // namespace emio::detail
