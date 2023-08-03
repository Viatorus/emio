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

class format_parser final : public parser_base<input_validation::disabled> {
 public:
  constexpr explicit format_parser(writer& wtr, reader& format_rdr) noexcept
      : parser_base<input_validation::disabled>{format_rdr}, wtr_{wtr} {}

  format_parser(const format_parser&) = delete;
  format_parser(format_parser&&) = delete;
  format_parser& operator=(const format_parser&) = delete;
  format_parser& operator=(format_parser&&) = delete;
  constexpr ~format_parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char c) noexcept override {
    return wtr_.write_char(c);
  }

  result<void> apply(uint8_t arg_nbr, const args_span<format_arg>& args) noexcept {
    return args.get_args()[arg_nbr].format(wtr_, format_rdr_);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> apply(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> apply(uint8_t arg_pos, const Arg& arg, const Args&... args) noexcept {
    if (arg_pos == 0) {
      return write_arg(arg);
    }
    return apply(arg_pos - 1, args...);
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

  writer& wtr_;
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
constexpr format_parser::~format_parser() noexcept = default;

class format_specs_checker final : public parser_base<input_validation::enabled> {
 public:
  using parser_base<input_validation::enabled>::parser_base;

  format_specs_checker(const format_specs_checker& other) = delete;
  format_specs_checker(format_specs_checker&& other) = delete;
  format_specs_checker& operator=(const format_specs_checker& other) = delete;
  format_specs_checker& operator=(format_specs_checker&& other) = delete;
  constexpr ~format_specs_checker() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char /*c*/) noexcept override {
    return success;
  }

  result<void> apply(uint8_t arg_nbr, const args_span<format_validation_arg>& args) noexcept {
    return args.get_args()[arg_nbr].validate(this->format_rdr_);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  template <typename... Args>
    requires(sizeof...(Args) == 0)
  constexpr result<void> apply(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> apply(uint8_t arg_pos, std::type_identity<Arg> /*unused*/, Args... args) noexcept {
    if (arg_pos == 0) {
      return validate_arg<Arg>();
    }
    return apply(arg_pos - 1, args...);
  }

 private:
  template <typename Arg>
  constexpr result<void> validate_arg() noexcept {
    return validate_for<std::remove_cvref_t<Arg>>(this->format_rdr_);
  }
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
constexpr format_specs_checker::~format_specs_checker() noexcept = default;

template <typename... Args>
[[nodiscard]] constexpr bool validate_format_string(std::string_view format_str) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    return validate<format_specs_checker>(format_str, sizeof...(Args), std::type_identity<Args>{}...);
  } else {
    return validate<format_specs_checker>(format_str, sizeof...(Args),
                                          make_validation_args<format_validation_arg, Args...>(format_str));
  }
}

}  // namespace emio::detail::format
