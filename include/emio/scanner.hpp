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
   * @param spec_rdr The reader of the scan spec.
   * @return Success if the scan spec is valid.
   */
  static constexpr result<void> validate(reader& spec_rdr) noexcept {
    return spec_rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the scan specs for this type.
   * @param spec_rdr The reader of the scan spec.
   * @return Success if the scan spec is valid and could be parsed.
   */
  constexpr result<void> parse(reader& spec_rdr) noexcept {
    return spec_rdr.read_if_match_char('}');
  }

  /**
   * Function to scan the object of this type according to the parsed scan specs.
   * @param in The input reader.
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
  static constexpr result<void> validate(reader& spec_rdr) noexcept {
    detail::scan::scan_specs specs{};
    EMIO_TRYV(detail::scan::validate_scan_specs(spec_rdr, specs));
    if constexpr (std::is_same_v<T, char>) {
      EMIO_TRYV(check_char_specs(specs));
    } else if constexpr (std::is_integral_v<T>) {
      EMIO_TRYV(check_integral_specs(specs));
    } else {
      static_assert(detail::always_false_v<T>, "Unknown core type!");
    }
    return success;
  }

  constexpr result<void> parse(reader& spec_rdr) noexcept {
    return detail::scan::parse_scan_specs(spec_rdr, specs_);
  }

  constexpr result<void> scan(reader& in, T& arg) const noexcept {
    return read_arg(in, specs_, arg);
  }

 private:
  detail::scan::scan_specs specs_{};
};

/**
 * Scanner for std::string_view.
 */
template <>
class scanner<std::string_view> {
 public:
  static constexpr result<void> validate(reader& spec_rdr) noexcept {
    detail::scan::scan_specs specs{};
    EMIO_TRYV(detail::scan::validate_scan_specs(spec_rdr, specs));
    EMIO_TRYV(detail::scan::check_string_specs(specs));
    return success;
  }

  constexpr result<void> parse(reader& spec_rdr) noexcept {
    EMIO_TRYV(detail::scan::parse_scan_specs(spec_rdr, specs_));
    spec_rdr_ = spec_rdr;
    return success;
  }

  constexpr result<void> scan(reader& in, std::string_view& arg) noexcept {
    return detail::scan::read_string(in, specs_, spec_rdr_, arg);
  }

 private:
  detail::scan::scan_specs specs_;
  reader spec_rdr_;
};

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

}  // namespace emio
