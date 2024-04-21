//
// Copyright (c) 2024 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <exception>
#if __STDC_HOSTED__
#  include <filesystem>
#endif
#include <optional>
#include <variant>

#include "formatter.hpp"

namespace emio {

/**
 * Formatter for std::optional.
 * @tparam T The type to format.
 */
template <typename T>
  requires is_formattable_v<T>
class formatter<std::optional<T>> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return detail::format::validate_trait<T>(format_rdr);
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    return underlying_.parse(format_rdr);
  }

  constexpr result<void> format(writer& out, const std::optional<T>& arg) const noexcept {
    if (!arg.has_value()) {
      return out.write_str(detail::sv("none"));
    } else {
      EMIO_TRYV(out.write_str(detail::sv("optional(")));
      EMIO_TRYV(underlying_.format(out, *arg));
      return out.write_char(')');
    }
  }

 private:
  formatter<T> underlying_;
};

/**
 * Formatter for std::exception.
 * @tparam T The type.
 */
template <typename T>
  requires std::is_base_of_v<std::exception, T>
class formatter<T> : public formatter<std::string_view> {
 public:
  result<void> format(writer& out, const std::exception& arg) const noexcept {
    return formatter<std::string_view>::format(out, arg.what());
  }
};

#if __STDC_HOSTED__
/**
 * Formatter for std::filesystem::path.
 */
template <>
class formatter<std::filesystem::path> : public formatter<std::string_view> {
 public:
  result<void> format(writer& out, const std::filesystem::path& arg) const noexcept {
    return formatter<std::string_view>::format(out, arg.native());
  }
};
#endif

/**
 * Formatter for std::monostate.
 */
template <>
class formatter<std::monostate> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): API requests this to be a member function.
  constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): API requests this to be a member function.
  constexpr result<void> format(writer& out, const std::monostate& /*arg*/) const noexcept {
    return out.write_str(detail::sv("monostate"));
  }
};

/**
 * Formatter for std::variant.
 * @tparam Ts The types to format.
 */
template <typename... Ts>
  requires(is_formattable_v<Ts> && ...)
class formatter<std::variant<Ts...>> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  constexpr result<void> format(writer& out, const std::variant<Ts...>& arg) noexcept {
    EMIO_TRYV(out.write_str(detail::sv("variant(")));
#ifdef __EXCEPTIONS
    try {
#endif
      EMIO_TRYV(std::visit(detail::overload{
                               [&](const char& val) -> result<void> {
                                 return out.write_char_escaped(val);
                               },
                               [&](const std::string_view& val) -> result<void> {
                                 return out.write_str_escaped(val);
                               },
                               [&](const auto& val) -> result<void> {
                                 using T = std::decay_t<decltype(val)>;
                                 if constexpr (!std::is_null_pointer_v<T> &&
                                               std::is_constructible_v<std::string_view, T>) {
                                   return out.write_str_escaped(std::string_view{val});
                                 } else {
                                   formatter<T> fmt;
                                   emio::reader rdr{detail::sv("}")};
                                   EMIO_TRYV(fmt.parse(rdr));
                                   return fmt.format(out, val);
                                 }
                               },
                           },
                           arg));
#ifdef __EXCEPTIONS
    } catch (const std::bad_variant_access&) {
      EMIO_TRYV(out.write_str(detail::sv("valueless by exception")));
    }
#endif
    return out.write_char(')');
  }  // namespace emio
};

}  // namespace emio
