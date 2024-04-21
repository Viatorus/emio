//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/misc.hpp"
#include "detail/scan/scanner.hpp"

namespace emio {

/**
 * Checks if a type is scannable.
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool is_scannable_v = detail::scan::has_scanner_v<std::remove_cvref_t<T>>;

/**
 * Class template that defines scanning rules for a given type.
 * @note This class definition is just a mock-up. See other template specialization for a concrete scanning.
 * @tparam T The type to scan.
 */
template <typename T>
class scanner {
 public:
  // Not constructable because this is just a minimal example how to write a custom scanner.
  scanner() = delete;

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
   * Function to scan the object of this type according to the parsed format specs.
   * @param input The input reader.
   * @param arg The argument to scan.
   * @return Success if the scanning could be done.
   */
  constexpr result<void> scan(reader& in, T& arg) const noexcept {
    EMIO_TRY(arg, in.parse_int<T>());
    return success;
  }
};

/**
 * Scanner for most common unambiguity types.
 * This includes:
 * - char
 * - integral
 * To be implemented:
 * - floating-point types
 * @tparam T The type.
 */
template <typename T>
  requires(detail::scan::is_core_type_v<T>)
class scanner<T> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    detail::scan::format_specs specs{};
    EMIO_TRYV(detail::scan::validate_format_specs(format_rdr, specs));
    if constexpr (std::is_same_v<T, char>) {
      EMIO_TRYV(check_char_specs(specs));
    } else if constexpr (std::is_integral_v<T>) {
      EMIO_TRYV(check_integral_specs(specs));
    } else {
      static_assert(detail::always_false_v<T>, "Unknown core type!");
    }
    return success;
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    return detail::scan::parse_format_specs(format_rdr, specs_);
  }

  constexpr result<void> scan(reader& in, T& arg) const noexcept {
    return read_arg(in, specs_, arg);
  }

 private:
  detail::scan::format_specs specs_{};
};

/**
 * Scanner for integral types which are not core types.
 */
template <typename T>
  requires(std::is_integral_v<T> && !std::is_same_v<T, bool> && !detail::scan::is_core_type_v<T>)
class scanner<T> : public scanner<detail::upcasted_int_t<T>> {
 private:
  using upcasted_t = detail::upcasted_int_t<T>;

 public:
  constexpr result<void> scan(reader& in, T& arg) noexcept {
    upcasted_t val{};
    EMIO_TRYV(scanner<upcasted_t>::scan(in, val));
    if (val < std::numeric_limits<T>::min() || val > std::numeric_limits<T>::max()) {
      return err::out_of_range;
    }
    arg = static_cast<T>(val);
    return success;
  }
};

/**
 * Scanner for std::string_view.
 */
template <>
class scanner<std::string_view> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    detail::scan::format_specs specs{};
    EMIO_TRYV(detail::scan::validate_format_specs(format_rdr, specs));
    EMIO_TRYV(detail::scan::check_string_specs(specs));
    return success;
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    EMIO_TRYV(detail::scan::parse_format_specs(format_rdr, specs_));
    format_rdr_ = format_rdr;
    return success;
  }

  constexpr result<void> scan(reader& in, std::string_view& arg) noexcept {
    return detail::scan::read_string(in, specs_, format_rdr_, arg);
  }

 private:
  detail::scan::format_specs specs_;
  reader format_rdr_;
};

#if __STDC_HOSTED__
/**
 * Scanner for std::string.
 */
template <>
class scanner<std::string> : public scanner<std::string_view> {
 public:
  constexpr result<void> scan(reader& in, std::string& arg) noexcept {
    std::string_view s;
    EMIO_TRYV(scanner<std::string_view>::scan(in, s));
    arg = s;
    return success;
  }
};
#endif

}  // namespace emio
