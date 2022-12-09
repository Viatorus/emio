//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../format_string.hpp"
#include "../../reader.hpp"
#include "../../writer.hpp"
#include "args.hpp"
#include "parser.hpp"

namespace emio::detail::format {

// Non constexpr version.
template <typename Char>
result<void> vformat_to(buffer<Char>& buf, const basic_format_args<Char>& args) noexcept {
  EMIO_TRY(const std::basic_string_view<Char> str, args.get_format_str());
  reader<Char> format_rdr{str};
  writer<char> wtr{buf};
  format_parser<Char> fh{wtr, format_rdr};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return res.assume_error();
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (auto res = args.get_args()[arg_nbr].format(wtr, format_rdr); !res) {
      return res.assume_error();
    }
  }
  return success;
}

// Constexpr version.
template <typename Char, typename... Args>
constexpr result<void> format_to(buffer<Char>& buf, basic_format_string<Char, Args...> format_str,
                                 const Args&... args) noexcept {
  EMIO_TRY(std::basic_string_view<Char> str, format_str.get());
  reader<Char> format_rdr{str};
  writer<Char> wtr{buf};
  format_parser<Char> fh{wtr, format_rdr};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return res.assume_error();
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (auto res = fh.find_and_write_arg(arg_nbr, args...); !res) {
      return res.assume_error();
    }
  }
  return success;
}

}  // namespace emio::detail::format
