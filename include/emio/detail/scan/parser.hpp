//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../writer.hpp"
#include "../bitset.hpp"
#include "args.hpp"
#include "scanner.hpp"

namespace emio::detail::scan {

template <typename Input, typename CRTP>
class parser : public parser_base<input_validation::disabled> {
 public:
  constexpr explicit parser(Input& input, reader& format_rdr) noexcept
      : parser_base<input_validation::disabled>{format_rdr}, input_{input} {}

  parser(const parser&) = delete;
  parser(parser&&) = delete;
  parser& operator=(const parser&) = delete;
  parser& operator=(parser&&) = delete;
  constexpr ~parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  result<void> apply(uint8_t arg_nbr, const args_span<scan_arg>& args) noexcept {
    return static_cast<CRTP*>(this)->apply3(args.get_args()[arg_nbr]);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> apply(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> apply(uint8_t arg_pos, Arg& arg, Args&... args) noexcept {
    if (arg_pos == 0) {
      return static_cast<CRTP*>(this)->apply2(arg);
    }
    return apply(arg_pos - 1, args...);
  }

 protected:
  Input& input_;
};

// Explicit out-of-class definition because of GCC bug: ~format_parser() used before its definition.
template<typename Input, typename CRTP>
constexpr parser<Input, CRTP>::~parser() noexcept = default;

class scan_parser2 : public parser<reader, scan_parser2> {
 public:
  using parser<reader, scan_parser2>::parser;

  constexpr result<void> process(const char c) noexcept override {
    return input_.read_if_match_char(c);
  }

  template <typename Arg>
  constexpr result<void> apply3(Arg& arg) noexcept {
    return arg.scan(input_, format_rdr_);
  }

  template <typename Arg>
  constexpr result<void> apply2(Arg& arg) noexcept {
    if constexpr (has_scanner_v<Arg>) {
      scanner<Arg> scanner;
      EMIO_TRYV(scanner.parse(this->format_rdr_));
      return scanner.scan(input_, arg);
    } else {
      static_assert(has_scanner_v<Arg>,
                    "Cannot scan an argument. To make type T scannable provide a scanner<T> specialization.");
    }
  }
};

class scan_parser : public parser_base<input_validation::disabled> {
 public:
  constexpr explicit scan_parser(reader& input, reader& scan_rdr) noexcept
      : parser_base<input_validation::disabled>{scan_rdr}, input_{input} {}

  constexpr result<void> process(char c) noexcept override {
    return input_.read_if_match_char(c);
  }

  result<void> apply(uint8_t arg_nbr, const args_span<scan_arg>& args) noexcept {
    return args.get_args()[arg_nbr].scan(input_, format_rdr_);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> apply(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> apply(uint8_t arg_pos, Arg& arg, Args&... args) noexcept {
    if (arg_pos == 0) {
      return scan_arg(arg);
    }
    return apply(arg_pos - 1, args...);
  }

 private:
  template <typename Arg>
  constexpr result<void> scan_arg(Arg& arg) noexcept {
    if constexpr (has_scanner_v<Arg>) {
      scanner<Arg> scanner;
      EMIO_TRYV(scanner.parse(this->format_rdr_));
      return scanner.scan(input_, arg);
    } else {
      static_assert(has_scanner_v<Arg>,
                    "Cannot scan an argument. To make type T scannable provide a scanner<T> specialization.");
    }
  }

  reader& input_;
};

class scan_specs_checker final : public parser_base<input_validation::enabled> {
 public:
  using parser_base<input_validation::enabled>::parser_base;

  scan_specs_checker(const scan_specs_checker& other) = delete;
  scan_specs_checker(scan_specs_checker&& other) = delete;
  scan_specs_checker& operator=(const scan_specs_checker& other) = delete;
  scan_specs_checker& operator=(scan_specs_checker&& other) = delete;
  constexpr ~scan_specs_checker() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char /*c*/) noexcept override {
    return success;
  }

  result<void> apply(uint8_t arg_nbr, const args_span<scan_validation_arg>& args) noexcept {
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
constexpr scan_specs_checker::~scan_specs_checker() noexcept = default;

template <typename... Args>
[[nodiscard]] inline constexpr bool validate_scan_string(std::string_view scan_str) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    return validate<scan_specs_checker>(scan_str, sizeof...(Args), std::type_identity<Args>{}...);
  } else {
    return validate<scan_specs_checker>(scan_str, sizeof...(Args),
                                        make_validation_args<scan_validation_arg, Args...>(scan_str));
  }
}

}  // namespace emio::detail::scan
