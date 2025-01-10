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
#if defined(__cpp_lib_expected)
#  include <expected>
#endif

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

  static constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  static constexpr result<void> format(writer& out, const std::monostate& /*arg*/) noexcept {
    return out.write_str(detail::sv("monostate"));
  }
};

namespace detail {

template <typename T>
constexpr result<void> format_escaped_alternative(writer& out, const T& val) noexcept {
  if constexpr (std::is_same_v<T, char>) {
    return out.write_char_escaped(val);
  } else if constexpr (std::is_same_v<T, std::string_view>) {
    return out.write_str_escaped(val);
  } else if constexpr (!std::is_null_pointer_v<T> && std::is_constructible_v<std::string_view, T>) {
    return out.write_str_escaped(std::string_view{val});
  } else {
    formatter<T> fmt;
    emio::reader rdr{detail::sv("}")};
    EMIO_TRYV(fmt.parse(rdr));
    return fmt.format(out, val);
  }
}

}  // namespace detail

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
      EMIO_TRYV(std::visit([&out](const auto& val) -> result<void> {
        return detail::format_escaped_alternative(out, val);
      }, arg));
#ifdef __EXCEPTIONS
    } catch (const std::bad_variant_access&) {
      EMIO_TRYV(out.write_str(detail::sv("valueless by exception")));
    }
#endif
    return out.write_char(')');
  }
};

#if defined(__cpp_lib_expected)
/**
 * Formatter for std::expected.
 * @tparam T The value type.
 * @tparam E The error type.
 */
template <typename T, typename E>
  requires((!std::is_void_v<T> && is_formattable_v<T> || std::is_void_v<T>) && is_formattable_v<E>)
class formatter<std::expected<T, E>> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  constexpr result<void> format(writer& out, const std::expected<T, E>& arg) const noexcept {
    if (arg.has_value()) {
      EMIO_TRYV(out.write_str(detail::sv("expected(")));
      if constexpr (!std::is_void_v<T>) {
        EMIO_TRYV(out.write_str(detail::sv("void")));
      } else {
        EMIO_TRYV(detail::format_escaped_alternative(out, arg.value()));
      }
    } else {
      EMIO_TRYV(out.write_str(detail::sv("unexpected(")));
      EMIO_TRYV(detail::format_escaped_alternative(out, arg.error()));
    }
    return out.write_char(')');
  }
};
#endif

}  // namespace emio
