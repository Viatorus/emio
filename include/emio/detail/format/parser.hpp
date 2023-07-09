//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../writer.hpp"
#include "../bitset.hpp"
#include "args.hpp"
#include "formatter.hpp"

namespace emio::detail::format {

template <typename Char>
class format_parser final : public parser_base<Char, input_validation::disabled> {
 public:
  explicit constexpr format_parser(writer<Char>& wtr, reader<Char>& format_rdr) noexcept
      : parser_base<Char, input_validation::disabled>{format_rdr}, wtr_{wtr} {}

  format_parser(const format_parser&) = delete;
  format_parser(format_parser&&) = delete;
  format_parser& operator=(const format_parser&) = delete;
  format_parser& operator=(format_parser&&) = delete;
  constexpr ~format_parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char c) noexcept override {
    return wtr_.write_char(c);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> find_and_write_arg(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> find_and_write_arg(uint8_t arg_pos, const Arg& arg, const Args&... args) noexcept {
    if (arg_pos == 0) {
      return write_arg(arg);
    }
    return find_and_write_arg(arg_pos - 1, args...);
  }

 private:
  template <typename Arg>
  constexpr result<void> write_arg(const Arg& arg) noexcept {
    if constexpr (has_formatter_v<Arg>) {
      formatter<Arg> formatter;
      EMIO_TRYV(formatter.parse(this->format_rdr_));
      return formatter.format(wtr_, arg);
    } else {
      static_assert(has_formatter_v<Arg>,
                    "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
    }
  }

  writer<Char>& wtr_;
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
template <typename Char>
constexpr format_parser<Char>::~format_parser() noexcept = default;

template <typename Char>
class format_specs_checker final : public parser_base<Char, input_validation::enabled> {
 public:
  using parser_base<Char, input_validation::enabled>::parser_base;

  format_specs_checker(const format_specs_checker& other) = delete;
  format_specs_checker(format_specs_checker&& other) = delete;
  format_specs_checker& operator=(const format_specs_checker& other) = delete;
  format_specs_checker& operator=(format_specs_checker&& other) = delete;
  constexpr ~format_specs_checker() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char /*c*/) noexcept override {
    return success;
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  template <typename... Args>
    requires(sizeof...(Args) == 0)
  constexpr result<void> find_and_validate_arg(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> find_and_validate_arg(uint8_t arg_pos) noexcept {
    if (arg_pos == 0) {
      return validate_arg<Arg>();
    }
    return find_and_validate_arg<Args...>(arg_pos - 1);
  }

 private:
  template <typename Arg>
  constexpr result<void> validate_arg() noexcept {
    return validate_for<std::remove_cvref_t<Arg>>(this->format_rdr_);
  }
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
template <typename Char>
constexpr format_specs_checker<Char>::~format_specs_checker() noexcept = default;

template <typename Char>
[[nodiscard]] bool validate_format_string_fallback(const format_validation_args<Char>& args) noexcept {
  reader<Char> format_rdr{args.get_format_str()};
  format_specs_checker<Char> fh{format_rdr};
  bitset<128> matched{};
  const size_t arg_cnt = args.get_args().size();
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return false;
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (arg_cnt <= arg_nbr) {
      return false;
    }
    matched.set(arg_nbr);
    auto res = args.get_args()[arg_nbr].validate(format_rdr);
    if (!res) {
      return false;
    }
  }
  return matched.all_first(arg_cnt);
}

template <typename... Args, typename Char>
[[nodiscard]] constexpr bool validate_format_string(std::basic_string_view<Char> format_str) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    reader<Char> format_rdr{format_str};
    format_specs_checker<Char> fh{format_rdr};
    bitset<sizeof...(Args)> matched{};
    while (true) {
      uint8_t arg_nbr{detail::no_more_args};
      if (auto res = fh.parse(arg_nbr); !res) {
        return false;
      }
      if (arg_nbr == detail::no_more_args) {
        break;
      }
      if (matched.size() <= arg_nbr) {
        return false;
      }
      matched.set(arg_nbr);
      auto res = fh.template find_and_validate_arg<Args...>(arg_nbr);
      if (!res) {
        return false;
      }
    }
    return matched.all();
  } else {
    return validate_format_string_fallback(make_format_validation_args<Char, Args...>(format_str));
  }
}

}  // namespace emio::detail::format
