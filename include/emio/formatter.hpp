//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <memory>

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
   * Optional static function to validate the format string syntax for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param format_rdr The reader over the format string.
   * @return Success if the format string is valid.
   */
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param format_rdr The reader over the format string.
   * @return Success if the format string is valid and could be parsed.
   */
  constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to format the object of this type according to the parsed format specs.
   * @param out The output writer.
   * @param arg The argument to format.
   * @return Success if the formatting could be done.
   */
  constexpr result<void> format(writer& out, const T& arg) const noexcept {
    return out.write_int(sizeof(arg));
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
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    detail::format::format_specs specs{};
    EMIO_TRYV(validate_format_specs(format_rdr, specs));
    if constexpr (std::is_same_v<T, bool>) {
      EMIO_TRYV(check_bool_specs(specs));
    } else if constexpr (std::is_same_v<T, char>) {
      EMIO_TRYV(check_char_specs(specs));
    } else if constexpr (detail::format::is_void_pointer_v<T> || std::is_null_pointer_v<T>) {
      EMIO_TRYV(check_pointer_specs(specs));
    } else if constexpr (std::is_integral_v<T>) {
      EMIO_TRYV(check_integral_specs(specs));
      if constexpr (std::is_unsigned_v<T>) {
        EMIO_TRYV(check_unsigned_specs(specs));
      }
    } else if constexpr (std::is_floating_point_v<T>) {
      EMIO_TRYV(check_floating_point_specs(specs));
    } else if constexpr (std::is_constructible_v<std::string_view, T>) {
      EMIO_TRYV(check_string_specs(specs));
    } else {
      static_assert(detail::always_false_v<T>, "Unknown core type!");
    }
    return success;
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    return detail::format::parse_format_specs(format_rdr, specs_);
  }

  constexpr result<void> format(writer& out, const T& arg) const noexcept {
    auto specs = specs_;  // Copy spec because format could be called multiple times (e.g. ranges).
    return write_arg(out, specs, arg);
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
   * Sets the width.
   * @note Used e.g. from dynamic spec formatter.
   * @param width The width.
   */
  constexpr void set_width(int32_t width) noexcept {
    specs_.width = std::max<int32_t>(0, width);
  }

  /**
   * Sets the precision.
   * @note Used e.g. from dynamic spec formatter.
   * @param precision The precision.
   */
  constexpr void set_precision(int32_t precision) noexcept {
    specs_.precision = std::max<int32_t>(0, precision);
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
  constexpr result<void> format(writer& out, const T& arg) const noexcept {
    return formatter<std::underlying_type_t<T>>::format(out, static_cast<std::underlying_type_t<T>>(arg));
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
  constexpr result<void> format(writer& out, const T& arg) const noexcept {
    return formatter<detail::format::format_as_return_t<T>>::format(out, format_as(arg));
  }
};

namespace detail {

template <typename T>
struct format_spec_with_value;

}

/**
 * Struct to dynamically specify width and precision.
 */
struct format_spec {
  /// Constant which indicates that the spec should not overwrite the parsed spec from the format string.
  static constexpr int32_t not_defined = -std::numeric_limits<int32_t>::max();

  /// The width.
  int32_t width{not_defined};
  /// The precision.
  int32_t precision{not_defined};

  /**
   * Returns an object that holds the value and the dynamic specification as reference.
   *
   * @note The object uses reference semantics and does not extend the lifetime of the held objects. It is the
   * programmer's responsibility to ensure that value outlive the return value. Usually, the result is only used as
   * argument to a formatting function.
   *
   * @param value The value to format.
   * @return Internal type.
   */
  template <typename T>
  [[nodiscard]] constexpr detail::format_spec_with_value<T> with(const T& value) const noexcept;
};

namespace detail {

/**
 * Struct holding the format spec and value.
 */
template <typename T>
struct format_spec_with_value {
  const format_spec& spec;
  const T& value;
};

}  // namespace detail

template <typename T>
[[nodiscard]] constexpr detail::format_spec_with_value<T> format_spec::with(const T& value) const noexcept {
  return {*this, value};
}

/**
 * Formatter for types whose format specification is dynamically defined.
 * @tparam T The underlying type.
 */
template <typename T>
class formatter<detail::format_spec_with_value<T>> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return formatter<T>::validate(format_rdr);
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    return underlying_.parse(format_rdr);
  }

  constexpr result<void> format(writer& out, const detail::format_spec_with_value<T>& arg) noexcept {
    overwrite_spec(arg.spec);
    return underlying_.format(out, arg.value);
  }

 private:
  template <typename F>
    requires requires(F x) {
      x.set_width(1);
      x.set_precision(1);
    }
  static constexpr F& get_core_formatter(F& formatter) noexcept {
    return formatter;
  }

  template <typename F>
    requires requires(F x) { x.underlying(); }
  static constexpr auto& get_core_formatter(F& formatter) noexcept {
    return get_core_formatter(formatter.underlying());
  }

  constexpr void overwrite_spec(const format_spec& spec) noexcept {
    // Overwrite the spec of the core formatter if they are dynamically defined.
    auto& f = get_core_formatter(underlying_);
    if (spec.width != format_spec::not_defined) {
      f.set_width(spec.width);
    }
    if (spec.precision != format_spec::not_defined) {
      f.set_precision(spec.precision);
    }
  }

  formatter<T> underlying_{};
};

/**
 * Converts a value of a pointer-like type to const void * for pointer formatting.
 * @param p The value of the pointer.
 * @return The const void* version of the pointer.
 */
template <typename T>
  requires(std::is_pointer_v<T>)
constexpr auto ptr(T p) noexcept {
  if constexpr (std::is_volatile_v<std::remove_pointer_t<T>>) {
    return static_cast<const volatile void*>(p);
  } else {
    return static_cast<const void*>(p);
  }
}

template <typename T, typename Deleter>
constexpr const void* ptr(const std::unique_ptr<T, Deleter>& p) {
  return p.get();
}

#if __STDC_HOSTED__
template <typename T>
const void* ptr(const std::shared_ptr<T>& p) {
  return p.get();
}
#endif

}  // namespace emio
