//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <array>
#include <span>
#include <string_view>

#include "../../formatter.hpp"

namespace emio::detail::format {

// To reduce code bloat, similar types are unified to a general one.
template <typename T>
struct unified_type;

template <typename T>
struct unified_type {
  using type = const T&;
};

template <typename T>
  requires(!std::is_integral_v<T> && !std::is_same_v<T, std::nullptr_t> && std::is_constructible_v<std::string_view, T>)
struct unified_type<T> {
  using type = std::string_view;
};

template <typename T>
  requires(std::is_integral_v<T> && std::is_signed_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
struct unified_type<T> {
  using type = std::conditional_t<num_bits<T>() <= 32, int32_t, int64_t>;
};

template <typename T>
  requires(std::is_same_v<T, char> || std::is_same_v<T, bool> || std::is_same_v<T, void*> ||
           std::is_same_v<T, std::nullptr_t>)
struct unified_type<T> {
  using type = T;
};

template <typename T>
  requires(std::is_integral_v<T> && std::is_unsigned_v<T> && !std::is_same_v<T, bool> && !std::is_same_v<T, char>)
struct unified_type<T> {
  using type = std::conditional_t<num_bits<T>() <= 32, uint32_t, uint64_t>;
};

template <typename T>
using unified_type_t = typename unified_type<T>::type;

/**
 * Type erased format argument just for format string validation.
 */
class format_validation_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit format_validation_arg(std::type_identity<T> /*unused*/) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<unified_type_t<T>>*>(&storage_));
  }

  format_validation_arg(const format_validation_arg&) = delete;
  format_validation_arg(format_validation_arg&&) = delete;
  format_validation_arg& operator=(const format_validation_arg&) = delete;
  format_validation_arg& operator=(format_validation_arg&&) = delete;
  // No destructor & delete call to concept_t because model_t holds only a reference.
  ~format_validation_arg() = default;

  result<void> validate(reader<char>& format_is) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->validate(format_is);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> validate(reader<char>& format_is) const noexcept = 0;

   protected:
    ~concept_t() = default;
  };

  template <typename T>
  class model_t final : public concept_t {
   public:
    explicit model_t() noexcept = default;
    model_t(const model_t&) = delete;
    model_t(model_t&&) = delete;
    model_t& operator=(const model_t&) = delete;
    model_t& operator=(model_t&&) = delete;

    result<void> validate(reader<char>& format_is) const noexcept override {
      return validate_for<std::decay_t<T>>(format_is);
    }

   protected:
    ~model_t() = default;
  };

  std::aligned_storage_t<sizeof(model_t<int>)> storage_;
};

/**
 * Format arguments just for format string validation.
 */
template <typename Char>
class format_validation_args {
 public:
  format_validation_args(const format_validation_args&) = delete;
  format_validation_args(format_validation_args&&) = delete;
  format_validation_args& operator=(const format_validation_args&) = delete;
  format_validation_args& operator=(format_validation_args&&) = delete;
  ~format_validation_args() = default;

  [[nodiscard]] std::basic_string_view<Char> get_format_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const format_validation_arg> get_args() const noexcept {
    return args_;
  }

 protected:
  format_validation_args(std::basic_string_view<Char> format_str, std::span<const format_validation_arg> args)
      : format_str_{format_str}, args_{args} {}

 private:
  std::basic_string_view<Char> format_str_;
  std::span<const detail::format::format_validation_arg> args_;
};

/**
 * Format arguments storage just for format string validation.
 */
template <typename Char, size_t NbrOfArgs>
class basic_format_validation_args_storage : public format_validation_args<Char> {
 public:
  template <typename... Args>
  basic_format_validation_args_storage(std::basic_string_view<Char> str, const Args&... args)
      : format_validation_args<Char>{str, args_storage_}, args_storage_{format_validation_arg{args}...} {}

  basic_format_validation_args_storage(const basic_format_validation_args_storage&) = delete;
  basic_format_validation_args_storage(basic_format_validation_args_storage&&) = delete;
  basic_format_validation_args_storage& operator=(const basic_format_validation_args_storage&) = delete;
  basic_format_validation_args_storage& operator=(basic_format_validation_args_storage&&) = delete;
  ~basic_format_validation_args_storage() = default;

 private:
  std::array<format_validation_arg, NbrOfArgs> args_storage_;
};

template <typename Char, typename... Args>
basic_format_validation_args_storage<Char, sizeof...(Args)> make_format_validation_args(
    std::basic_string_view<Char> format_str) {
  return {format_str, std::type_identity<Args>{}...};
}

/**
 * Type erased format argument for formatting.
 */
template <typename Char>
class basic_format_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit basic_format_arg(const T& value) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<unified_type_t<T>>*>(&storage_), value);
  }

  basic_format_arg(const basic_format_arg&) = delete;
  basic_format_arg(basic_format_arg&&) = delete;
  basic_format_arg& operator=(const basic_format_arg&) = delete;
  basic_format_arg& operator=(basic_format_arg&&) = delete;
  ~basic_format_arg() = default;  // No destructor & delete call to concept_t because model_t holds only a reference.

  result<void> format(writer<Char>& wtr, reader<Char>& format_is) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->format(wtr, format_is);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> format(writer<Char>& wtr, reader<Char>& format_is) const noexcept = 0;

   protected:
    ~concept_t() = default;
  };

  template <typename T>
  class model_t final : public concept_t {
   public:
    explicit model_t(T value) noexcept : value_{value} {}

    model_t(const model_t&) = delete;
    model_t(model_t&&) = delete;
    model_t& operator=(const model_t&) = delete;
    model_t& operator=(model_t&&) = delete;

    result<void> format(writer<Char>& wtr, reader<Char>& format_is) const noexcept override {
      formatter<std::decay_t<T>> formatter;
      EMIO_TRYV(invoke_formatter_parse<input_validation::disabled>(formatter, format_is));
      return formatter.format(wtr, value_);
    }

   protected:
    ~model_t() = default;

   private:
    T value_;
  };

  std::aligned_storage_t<sizeof(model_t<std::string_view>)> storage_;
};

/**
 * Format arguments for formatting.
 */
template <typename Char>
class basic_format_args {
 public:
  basic_format_args(const basic_format_args&) = delete;
  basic_format_args(basic_format_args&&) = delete;
  basic_format_args& operator=(const basic_format_args&) = delete;
  basic_format_args& operator=(basic_format_args&&) = delete;
  ~basic_format_args() = default;

  result<std::basic_string_view<Char>> get_format_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const basic_format_arg<Char>> get_args() const noexcept {
    return args_;
  }

 protected:
  basic_format_args(result<std::basic_string_view<Char>> format_str, std::span<const basic_format_arg<Char>> args)
      : format_str_{format_str}, args_{args} {}

 private:
  result<std::basic_string_view<Char>> format_str_;
  std::span<const basic_format_arg<Char>> args_;
};

/**
 * Format arguments storage for formatting.
 */
template <typename Char, size_t NbrOfArgs>
class basic_format_args_storage : public basic_format_args<Char> {
 public:
  template <typename... Args>
  basic_format_args_storage(result<std::basic_string_view<Char>> str, const Args&... args)
      : basic_format_args<Char>{str, args_storage_}, args_storage_{basic_format_arg<Char>{args}...} {}

  basic_format_args_storage(const basic_format_args_storage&) = delete;
  basic_format_args_storage(basic_format_args_storage&&) = delete;
  basic_format_args_storage& operator=(const basic_format_args_storage&) = delete;
  basic_format_args_storage& operator=(basic_format_args_storage&&) = delete;
  ~basic_format_args_storage() = default;

 private:
  std::array<basic_format_arg<Char>, NbrOfArgs> args_storage_;
};

}  // namespace emio::detail::format
