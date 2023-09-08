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
   * Optional static function to validate the scan specs for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param rdr The scan reader.
   * @return Success if the scan spec is valid.
   */
  static constexpr result<void> validate(reader& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the scan specs for this type.
   * @param rdr The scan reader.
   * @return Success if the scan spec is valid and could be parsed.
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
  static constexpr result<void> validate(reader& rdr) noexcept {
    detail::scan::scan_specs specs{};
    EMIO_TRYV(detail::scan::validate_scan_specs(rdr, specs));
    if constexpr (std::is_same_v<T, char>) {
      EMIO_TRYV(check_char_specs(specs));
    } else if constexpr (std::is_integral_v<T>) {
      EMIO_TRYV(check_integral_specs(specs));
    } else {
      static_assert(detail::always_false_v<T>, "Unknown core type!");
    }
    return success;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    return detail::scan::parse_scan_specs(rdr, specs_);
  }

  constexpr result<void> scan(reader& input, T& arg) const noexcept {
    return read_arg(input, specs_, arg);
  }

 private:
  detail::scan::scan_specs specs_{};
};

template <>
class scanner<std::string_view> {
 public:
  static constexpr result<void> validate(reader& rdr) noexcept {
    detail::scan::scan_string_specs specs{};
    EMIO_TRYV(detail::scan::validate_scan_specs(rdr, specs));
    if (specs.width == detail::scan::no_width) {
      reader remaining = rdr;
      if (remaining.read_if_match_char('{') && remaining.read_if_match_char('{')) {
        // Complex part is not implemented yet.
        return err::invalid_format;
      }
    }
    EMIO_TRYV(detail::scan::check_string_specs(specs));
    return success;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    EMIO_TRYV(detail::scan::parse_scan_specs(rdr, specs_));
    specs_.remaining = rdr;
    return success;
  }

  constexpr result<void> scan(reader& input, std::string_view& arg) noexcept {
    return detail::scan::read_arg(input, specs_, arg);
  }

 private:
  detail::scan::scan_string_specs specs_;
};

template <>
class scanner<std::string> : public scanner<std::string_view> {
 public:
  constexpr result<void> scan(reader& input, std::string& arg) noexcept {
    std::string_view s;
    EMIO_TRYV(scanner<std::string_view>::scan(input, s));
    arg = s;
    return success;
  }
};

}  // namespace emio
