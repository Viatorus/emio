//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "detail/scan/scanner.hpp"

namespace emio {

/**
 * Checks if a type is formattable.
 * @tparam T The type to check.
 */
template <typename T>
inline constexpr bool is_scannable_v = detail::scan::has_scanner_v<std::remove_cvref_t<T>>;

/**
 * Class template that defines scanning rules for a given type.
 * @note This class definition is just a mock-up. See other template specialization for a concrete scanning.
 * @tparam T The type to format.
 */
template <typename T>
class scanner {
 public:
  // Not constructable because this is just a minimal example how to write a custom formatter.
  scanner() = delete;

  /**
   * Optional static function to validate the scan specs for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param rdr The scan reader.
   * @return Success if the format spec is valid.
   */
  static constexpr result<void> validate(reader& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the scan specs for this type.
   * @param rdr The scan reader.
   * @return Success if the format spec is valid and could be parsed.
   */
  constexpr result<void> parse(reader& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to scan the object of this type according to the parsed scan specs.
   * @param input The input reader.
   * @param arg The argument to scan.
   * @return Success if the scanning could be done.
   */
  constexpr result<void> scan(reader& input, T& arg) const noexcept {
    EMIO_TRY(arg, input.parse_int<T>());
    return success;
  }
};

/**
 * Formatter for most common unambiguity types.
 * This includes:
 * - char
 * - integral
 * TBD:
 * - floating-point types
 * @tparam T The type.
 */
template <typename T>
  requires(detail::scan::is_core_type_v<T>)
class scanner<T> {
 public:
  static constexpr result<void> validate(reader& rdr) noexcept {
    EMIO_TRY(const char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> scan(reader& input, T& arg) const noexcept {
    return read_arg(input, specs_, arg);
  }

  detail::scan::scan_specs specs_{};
};

}  // namespace emio