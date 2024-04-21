//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>
#include <type_traits>

#include "parser.hpp"
#include "validated_string_storage.hpp"

namespace emio {

namespace detail {

/**
 * This class represents a not yet validated format/scan string, which has to be validated at runtime.
 */
class runtime_string {
 public:
  /**
   * Constructs an empty runtime format/scan string.
   */
  constexpr runtime_string() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr runtime_string(std::nullptr_t) = delete;
  constexpr runtime_string(int) = delete;
#if __STDC_HOSTED__
  // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved): as intended
  constexpr runtime_string(std::string&&) = delete;
#endif

  /**
   * Constructs the runtime format/scan string from any suitable char sequence.
   * @param str The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  constexpr explicit runtime_string(const S& str) : str_{str} {}

  /**
   * Obtains a view over the runtime format/scan string.
   * @return The view.
   */
  [[nodiscard]] constexpr std::string_view get() const noexcept {
    return str_;
  }

 private:
  std::string_view str_;
};

template <typename Trait, typename... Args>
class valid_string;

/**
 * This class represents a validated format/scan string. The format/scan string is either valid or not.
 * @note The validation happens at object construction.
 * @tparam Args The argument types to format.
 */
template <typename Trait, typename... Args>
class validated_string : public validated_string_storage {
 public:
  /**
   * Constructs and validates the format/scan string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format/scan string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval validated_string(const S& s) noexcept
      // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-array-to-pointer-decay): can construct a std::string_view
      : validated_string_storage{validated_string_storage::from<Trait, Args...>(s)} {
    if (get().has_error()) {
      std::terminate();
    }
  }

  /**
   * Constructs and validates a runtime format/scan string at runtime.
   * @param s The runtime format/scan string.
   */
  constexpr validated_string(const runtime_string& s) noexcept
      : validated_string_storage{validated_string_storage::from<Trait, Args...>(s.get())} {}

  /**
   * Returns format/scan string as valid one.
   * @return The valid format/scan string or invalid_format if the validation failed.
   */
  // NOLINTNEXTLINE(modernize-use-nodiscard): result<...> is already declared with nodiscard
  constexpr result<valid_string<Trait, Args...>> as_valid() const noexcept {
    if (get().has_value()) {
      return valid_string<Trait, Args...>{*this};
    }
    return err::invalid_format;
  }

 protected:
  constexpr explicit validated_string(const validated_string_storage& str) noexcept : validated_string_storage{str} {}
};

/**
 * This class represents a validated format/scan string. The format/scan string can only be valid.
 * @tparam Args The argument types to format.
 */
template <typename Trait, typename... Args>
class valid_string : public validated_string<Trait, Args...> {
 public:
  /**
   * Constructs and validates a format/scan string at runtime.
   * @param s The format/scan string.
   * @return The valid format/scan string or invalid_format if the validation failed.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  static constexpr result<valid_string<Trait, Args...>> from(const S& s) noexcept {
    validated_string_storage storage = validated_string_storage::from<Trait, Args...>(s);
    if (storage.get().has_value()) {
      return valid_string{storage};
    }
    return err::invalid_format;
  }

  /**
   * Constructs and validates the format/scan string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format/scan string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval valid_string(const S& s) noexcept : validated_string<Trait, Args...>{s} {}

 private:
  friend class validated_string<Trait, Args...>;

  constexpr explicit valid_string(const validated_string_storage& str) noexcept
      : validated_string<Trait, Args...>{str} {}
};

}  // namespace detail

// Alias template types.
using runtime_string = detail::runtime_string;

/**
 * @brief Constructs a runtime string from a given format/scan string.
 * @param s The format/scan string.
 * @return The runtime string.
 */
inline constexpr runtime_string runtime(const std::string_view& s) noexcept {
  return runtime_string{s};
}

}  // namespace emio
