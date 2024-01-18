//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <exception>
#include <filesystem>
#include <optional>
#include <variant>

#include "formatter.hpp"

namespace emio {

/**
 * Formatter for optional.
 * @tparam T The type.
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
  result<void> format(writer& out, const std::exception& arg) noexcept {
    return formatter<std::string_view>::format(out, arg.what());
  }
};

/**
 * Formatter for filesystem::path.
 * @tparam T The type.
 */
template <>
class formatter<std::filesystem::path> : public formatter<std::string_view> {
 public:
  result<void> format(writer& out, const std::filesystem::path& arg) noexcept {
    return formatter<std::string_view>::format(out, arg.native());
  }
};

}  // namespace emio
