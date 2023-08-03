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

class scan_parser : public parser_base<input_validation::enabled> {
 public:
  constexpr explicit scan_parser(reader& input, reader& scan_rdr) noexcept
      : parser_base<input_validation::enabled>{scan_rdr}, input_{input} {}

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
      return success;
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
constexpr scan_specs_checker::~scan_specs_checker() noexcept = default;

[[nodiscard]] inline bool validate_scan_string_fallback(const args_span<scan_validation_arg>& args) noexcept {
  reader scan_rdr{args.get_str().value()};  // TODO: value() assume?
  scan_specs_checker fh{scan_rdr};
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
    auto res = args.get_args()[arg_nbr].validate(scan_rdr);
    if (!res) {
      return false;
    }
  }
  return matched.all_first(arg_cnt);
}

template <typename... Args>
[[nodiscard]] inline constexpr bool validate_scan_string(std::string_view scan_str) {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    reader format_rdr{scan_str};
    scan_specs_checker fh{format_rdr};
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
    return validate_scan_string_fallback(make_validation_args<scan_validation_arg, Args...>(scan_str));
  }
}

}  // namespace emio::detail::scan
