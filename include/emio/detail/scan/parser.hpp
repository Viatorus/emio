//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "args.hpp"
#include "scanner.hpp"

namespace emio::detail::scan {

class scan_parser final : public parser<scan_parser, input_validation::disabled> {
 public:
  constexpr explicit scan_parser(reader& input, reader& format_rdr) noexcept
      : parser<scan_parser, input_validation::disabled>{format_rdr}, input_{input} {}

  scan_parser(const scan_parser&) = delete;
  scan_parser(scan_parser&&) = delete;
  scan_parser& operator=(const scan_parser&) = delete;
  scan_parser& operator=(scan_parser&&) = delete;
  constexpr ~scan_parser() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(const char c) noexcept override {
    return input_.read_if_match_char(c);
  }

  result<void> process_arg(const scan_arg& arg) noexcept {
    return arg.process_arg(input_, format_rdr_);
  }

  template <typename Arg>
  constexpr result<void> process_arg(Arg& arg) noexcept {
    if constexpr (has_scanner_v<Arg>) {
      scanner<Arg> scanner;
      EMIO_TRYV(scanner.parse(this->format_rdr_));
      return scanner.scan(input_, arg);
    } else {
      static_assert(has_scanner_v<Arg>,
                    "Cannot scan an argument. To make type T scannable provide a scanner<T> specialization.");
    }
  }

 private:
  reader& input_;
};

// Explicit out-of-class definition because of GCC bug: <destructor> used before its definition.
constexpr scan_parser::~scan_parser() noexcept = default;

class scan_specs_checker final : public parser<scan_specs_checker, input_validation::enabled> {
 public:
  using parser<scan_specs_checker, input_validation::enabled>::parser;

  scan_specs_checker(const scan_specs_checker& other) = delete;
  scan_specs_checker(scan_specs_checker&& other) = delete;
  scan_specs_checker& operator=(const scan_specs_checker& other) = delete;
  scan_specs_checker& operator=(scan_specs_checker&& other) = delete;
  constexpr ~scan_specs_checker() noexcept override;  // NOLINT(performance-trivially-destructible): See definition.

  constexpr result<void> process(char /*c*/) noexcept override {
    return success;
  }

  result<void> process_arg(const scan_validation_arg& arg) noexcept {
    return arg.validate(this->format_rdr_);
  }

  template <typename Arg>
  constexpr result<void> process_arg(std::type_identity<Arg> /*unused*/) noexcept {
    return scan_arg_trait<std::remove_cvref_t<Arg>>::validate(this->format_rdr_);
  }
};

// Explicit out-of-class definition because of GCC bug: <destructor> used before its definition.
constexpr scan_specs_checker::~scan_specs_checker() noexcept = default;

}  // namespace emio::detail::scan
