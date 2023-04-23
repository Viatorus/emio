//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/format/formatter.hpp"

namespace emio {

/**
 * Class template that defines formatting rules for a given type.
 * @tparam T The type to format.
 */
template <typename T>
class formatter {
 public:
  // Not constructable because this is just a minimal example how to write a custom formatter.
  formatter() = delete;

  /**
   * Optional static function to validate the format spec for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param rdr The format reader.
   * @return Success if the format spec is valid.
   */
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param rdr The format reader.
   * @return Success if the format spec is valid and could be parsed.
   */
  constexpr result<void> parse(reader<char>& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to format the object of this type according to the parsed format specs.
   * @param wtr The output writer.
   * @param arg The argument to format.
   * @return Success if the formatting could be done.
   */
  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return wtr.write_int(sizeof(arg));
  }
};

/**
 * Formatter for most common unambiguity types.
 * This includes:
 * - boolean
 * - void* / nullptr
 * - integral types
 * - floating-point types (TODO)
 * - chrono duration (TODO)
 * - ranges of the above types (TODO)
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_core_type_v<T>)
class formatter<T> {
 public:
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
    detail::format::format_specs specs{};
    EMIO_TRYV(validate_format_specs(rdr, specs));
    if constexpr (std::is_same_v<T, bool>) {
      EMIO_TRYV(check_bool_specs(specs));
    } else if constexpr (std::is_same_v<T, char>) {
      EMIO_TRYV(check_char_specs(specs));
    } else if constexpr (std::is_same_v<T, std::nullptr_t> || std::is_same_v<T, void*>) {
      EMIO_TRYV(check_pointer_specs(specs));
    } else if constexpr (std::is_integral_v<T>) {
      EMIO_TRYV(check_integral_specs(specs));
      if constexpr (std::is_unsigned_v<T>) {
        EMIO_TRYV(check_unsigned_specs(specs));
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      EMIO_TRYV(check_floating_point_specs(specs));
    } else if constexpr (std::is_constructible_v<std::string_view, T>) {
      EMIO_TRYV(check_string_view_specs(specs));
    } else {
      static_assert(detail::always_false_v<T>, "Unknown core type!");
    }
    return success;
  }

  constexpr result<void> parse(reader<char>& rdr) noexcept {
    return detail::format::parse_format_specs(rdr, specs_);
  }

  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return write_arg(wtr, specs_, arg);
  }

 private:
  detail::format::format_specs specs_{};
};

/**
 * Formatter for unscoped enum types to there underlying type.
 * @tparam T The unscoped enum type.
 */
template <typename T>
  requires(std::is_enum_v<T> && std::is_convertible_v<T, std::underlying_type_t<T>>)
class formatter<T> : public formatter<std::underlying_type_t<T>> {
 public:
  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return formatter<std::underlying_type_t<T>>::format(wtr, static_cast<std::underlying_type_t<T>>(arg));
  }
};

/**
 * Formatter for types which can formatted with a format_as function when using ADL.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::has_format_as<T>)
class formatter<T> : public formatter<detail::format::format_as_return_t<T>> {
 public:
  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    return formatter<detail::format::format_as_return_t<T>>::format(wtr, format_as(arg));
  }
};

}  // namespace emio
