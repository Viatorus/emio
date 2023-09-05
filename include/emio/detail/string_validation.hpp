//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <string_view>
#include <type_traits>

#include "parser.hpp"

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
  constexpr runtime_string(std::string&&) = delete;
  constexpr runtime_string(std::nullptr_t) = delete;
  constexpr runtime_string(int) = delete;

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
  [[nodiscard]] constexpr std::string_view view() const noexcept {
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
class validated_string {
 public:
  /**
   * Constructs and validates the format/scan string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format/scan string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval validated_string(const S& s) noexcept {
    std::string_view str{s};
    if (Trait::template validate_string<Args...>(str)) {
      str_ = str;
    } else {
      // Invalid format/scan string detected. Stop compilation.
      std::terminate();
    }
  }

  /**
   * Constructs and validates a runtime format/scan string at runtime.
   * @param s The runtime format/scan string.
   */
  constexpr validated_string(runtime_string s) noexcept {
    std::string_view str{s.view()};
    if (Trait::template validate_string<Args...>(str)) {
      str_ = str;
    }
  }

  /**
   * Returns the validated format/scan string as view.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::string_view> get() const noexcept {
    return str_;
  }

  /**
   * Returns format/scan string as valid one.
   * @return The valid format/scan string or invalid_format if the validation failed.
   */
  constexpr result<valid_string<Trait, Args...>> as_valid() const noexcept {
    if (str_.has_value()) {
      return valid_string<Trait, Args...>{valid, str_.assume_value()};
    }
    return err::invalid_format;
  }

 protected:
  static constexpr struct valid_t {
  } valid{};

  constexpr explicit validated_string(valid_t /*unused*/, std::string_view s) noexcept : str_{s} {}

 private:
  result<std::string_view> str_{err::invalid_format};  ///< Validated string.
};

/**
 * This class represents a validated format/scan string. The format/scan string can only be valid.
 * @tparam Args The argument types to format.
 */
template <typename Trait, typename... Args>
class valid_string : public validated_string<Trait, Args...> {
 public:
  /**
   * Constructs and validates the format/scan string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format/scan string is invalid.
   * @param s The char sequence.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  consteval valid_string(const S& s) noexcept : validated_string<Trait, Args...>{s} {}

  /**
   * Constructs and validates a format/scan string at runtime.
   * @param s The format/scan string.
   * @return The valid format/scan string or invalid_format if the validation failed.
   */
  template <typename S>
    requires(std::is_constructible_v<std::string_view, S>)
  static constexpr result<valid_string<Trait, Args...>> from(const S& s) noexcept {
    std::string_view str{s};
    if (!Trait::template validate_string<Args...>(str)) {
      return err::invalid_format;
    }
    return valid_string{valid, str};
  }

 private:
  friend class validated_string<Trait, Args...>;

  using valid_t = typename validated_string<Trait, Args...>::valid_t;
  using validated_string<Trait, Args...>::valid;

  constexpr explicit valid_string(valid_t /*unused*/, std::string_view s) noexcept
      : validated_string<Trait, Args...>{valid, s} {}
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
