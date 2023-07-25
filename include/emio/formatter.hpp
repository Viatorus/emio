//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/format/formatter.hpp"

namespace emio {

/**
 * Checks if a type is formattable.
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool is_formattable_v = detail::format::has_formatter_v<std::remove_cvref_t<T>>;

/**
 * Class template that defines formatting rules for a given type.
 * @note This class definition is just a mock-up. See other template specialization for a concrete formatting.
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
 * - char
 * - string_view
 * - void* / nullptr
 * - integral, floating-point types
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
    auto specs = specs_;  // Copy spec because format could be called multiple times (e.g. ranges).
    return write_arg(wtr, specs, arg);
  }

  /**
   * Enables or disables the debug output format.
   * @note Used e.g. from range formatter.
   * @param enabled Flag to enable or disable the debug output.
   */
  constexpr void set_debug_format(bool enabled) noexcept
    requires(std::is_same_v<T, char> || std::is_same_v<T, std::string_view>)
  {
    if (enabled) {
      specs_.type = '?';
    } else {
      specs_.type = detail::format::no_type;
    }
  }

  /**
   *
   * @param width
   * @return
   */
  constexpr void set_width(int32_t width) noexcept {
    specs_.width = std::max(0, width);
  }

  /**
   * Set precision.
   * @param width
   * @return
   */
  constexpr void set_precision(int32_t precision) noexcept {
    specs_.precision = std::max(0, precision);
  }

 private:
  detail::format::format_specs specs_{};
};

/**
 * Formatter for any type which could be represented as a core type. E.g. string -> string_view.
 * @tparam T The unscoped enum type.
 */
template <typename T>
  requires(!detail::format::is_core_type_v<T> && detail::format::is_core_type_v<detail::format::unified_type_t<T>>)
class formatter<T> : public formatter<detail::format::unified_type_t<T>> {};

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

namespace detail {

template <typename T>
struct dynamic_spec_with_value;

}

struct dynamic_spec {
  int32_t width{detail::format::no_width_defined};
  int32_t precision{detail::format::no_precision_defined};

  template <typename T>
  [[nodiscard]] constexpr detail::dynamic_spec_with_value<T> with(const T& value) const noexcept;
};

namespace detail {

template <typename T>
struct dynamic_spec_with_value : dynamic_spec {
  const T& value;
};

}  // namespace detail

template <typename T>
[[nodiscard]] constexpr detail::dynamic_spec_with_value<T> dynamic_spec::with(const T& value) const noexcept {
  return {*this, value};
}

template <typename T>
class formatter<detail::dynamic_spec_with_value<T>> {
 public:
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
    return formatter<T>::validate(rdr);
  }

  constexpr result<void> parse(reader<char>& rdr) noexcept {
    return underlying_.parse(rdr);
  }

  constexpr result<void> format(writer<char>& wtr, const detail::dynamic_spec_with_value<T>& arg) noexcept {
    // Alter specs.
    if (arg.width != detail::format::no_width_defined) {
      underlying_.set_width(arg.width);
    }
    if (arg.precision != detail::format::no_precision_defined) {
      underlying_.set_precision(arg.precision);
    }
    return underlying_.format(wtr, arg.value);
  }

 private:
  formatter<T> underlying_{};
};

}  // namespace emio
