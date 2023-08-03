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
#include "../args.hpp"

namespace emio::detail::format {

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

  result<void> validate(reader& format_is) const noexcept {
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

    virtual result<void> validate(reader& format_is) const noexcept = 0;

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

    result<void> validate(reader& format_is) const noexcept override {
      return validate_for<std::remove_cvref_t<T>>(format_is);
    }

   protected:
    ~model_t() = default;
  };

  std::aligned_storage_t<sizeof(model_t<int>)> storage_;
};

/**
 * Type erased format argument for formatting.
 */
class format_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit format_arg(const T& value) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<unified_type_t<T>>*>(&storage_), value);
  }

  format_arg(const format_arg&) = delete;
  format_arg(format_arg&&) = delete;
  format_arg& operator=(const format_arg&) = delete;
  format_arg& operator=(format_arg&&) = delete;
  ~format_arg() = default;  // No destructor & delete call to concept_t because model_t holds only a reference.

  result<void> parse_and_format(writer& wtr, reader& format_is) const noexcept {
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

    virtual result<void> parse_and_format(writer& wtr, reader& format_is) const noexcept = 0;

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

    result<void> parse_and_format(writer& wtr, reader& format_is) const noexcept override {
      formatter<std::remove_cvref_t<T>> formatter;
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

}  // namespace emio::detail::format
