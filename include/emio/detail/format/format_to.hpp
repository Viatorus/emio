//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../../reader.hpp"
#include "../../writer.hpp"
#include "../validated_string.hpp"
#include "args.hpp"
#include "formatter.hpp"
#include "parser.hpp"

namespace emio {

namespace detail::format {

struct format_trait {
  template <typename... Args>
  [[nodiscard]] static constexpr bool validate_string(std::string_view format_str) noexcept {
    if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
      return validate<format_specs_checker>(format_str, sizeof...(Args), std::type_identity<Args>{}...);
    } else {
      return validate<format_specs_checker>(
          format_str, sizeof...(Args), related_format_args{make_validation_args<format_validation_arg, Args...>()});
    }
  }
};

template <typename... Args>
using format_string = validated_string<format_trait, std::type_identity_t<Args>...>;

template <typename... Args>
using valid_format_string = valid_string<format_trait, std::type_identity_t<Args>...>;

// Non constexpr version.
inline result<void> vformat_to(buffer& buf, const format_args& args) noexcept {
  EMIO_TRY(const std::string_view str, args.get_str());
  writer wtr{buf};
  if (args.is_plain_str()) {
    return wtr.write_str(str);
  }
  return parse<format_parser>(str, wtr, related_format_args{args});
}

// Constexpr version.
template <typename... Args>
constexpr result<void> format_to(buffer& buf, const format_string<Args...>& format_string,
                                 const Args&... args) noexcept {
  EMIO_TRY(const std::string_view str, format_string.get());
  writer wtr{buf};
  if (format_string.is_plain_str()) {
    return wtr.write_str(str);
  }
  return parse<format_parser>(str, wtr, args...);
}

}  // namespace detail::format

/**
 * Formatter for format_args.
 */
template <>
class formatter<detail::format::format_args> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  static constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  static result<void> format(writer& out, const detail::format::format_args& arg) noexcept {
    return detail::format::vformat_to(out.get_buffer(), arg);
  }

  static constexpr bool format_can_fail = true;
};

/**
 * Formatter for types which inherit from format_args.
 */
template <typename T>
  requires(std::is_base_of_v<detail::format::format_args, T>)
class formatter<T> : public formatter<detail::format::format_args> {};

namespace detail::format {

template <typename T>
  requires(std::is_base_of_v<detail::format::format_args, T>)
struct unified_type<T> {
  using type = const detail::format::format_args&;
};

}  // namespace detail::format

}  // namespace emio
