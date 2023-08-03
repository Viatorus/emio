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

class format_parser final : public parser<format_parser, input_validation::disabled> {
 public:
  constexpr explicit format_parser(writer& output, reader& format_rdr) noexcept
      : parser<format_parser, input_validation::disabled>{format_rdr}, output_{output} {}

  format_parser(const format_parser&) = delete;
  format_parser(format_parser&&) = delete;
  format_parser& operator=(const format_parser&) = delete;
  format_parser& operator=(format_parser&&) = delete;
  constexpr ~format_parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char c) noexcept override {
    return output_.write_char(c);
  }

  result<void> process_arg(const format_arg& arg) noexcept {
    return arg.parse_and_format(output_, format_rdr_);
  }

  template <typename Arg>
  constexpr result<void> process_arg(const Arg& arg) noexcept {
    if constexpr (has_formatter_v<Arg>) {
      formatter<Arg> formatter;
      EMIO_TRYV(formatter.parse(this->format_rdr_));
      return formatter.format(output_, arg);
    } else {
      static_assert(has_formatter_v<Arg>,
                    "Cannot format an argument. To make type T formattable provide a formatter<T> specialization.");
    }
  }

 private:
  writer& output_;
};

// Explicit out-of-class definition because of GCC bug: <destructor> used before its definition.
constexpr format_parser::~format_parser() noexcept = default;

class format_specs_checker final : public parser<format_specs_checker, input_validation::enabled> {
 public:
  using parser<format_specs_checker, input_validation::enabled>::parser;

  format_specs_checker(const format_specs_checker& other) = delete;
  format_specs_checker(format_specs_checker&& other) = delete;
  format_specs_checker& operator=(const format_specs_checker& other) = delete;
  format_specs_checker& operator=(format_specs_checker&& other) = delete;
  constexpr ~format_specs_checker() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char /*c*/) noexcept override {
    return success;
  }

  result<void> process_arg(const format_validation_arg& arg) noexcept {
    return arg.validate(this->format_rdr_);
  }

  template <typename Arg>
  constexpr result<void> process_arg(std::type_identity<Arg> /*unused*/) noexcept {
    return validate_for<std::remove_cvref_t<Arg>>(this->format_rdr_);
  }
};

// Explicit out-of-class definition because of GCC bug: <destructor> used before its definition.
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
