//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <array>
#include <span>
#include <string_view>

#include "../../scanner.hpp"
#include "../args.hpp"

namespace emio::detail::scan {

/**
 * Type erased scan argument just for scan string validation.
 */
class scan_validation_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit scan_validation_arg(std::type_identity<T> /*unused*/) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<T>*>(&storage_));
  }

  scan_validation_arg(const scan_validation_arg&) = delete;
  scan_validation_arg(scan_validation_arg&&) = delete;
  scan_validation_arg& operator=(const scan_validation_arg&) = delete;
  scan_validation_arg& operator=(scan_validation_arg&&) = delete;
  // No destructor & delete call to concept_t because model_t holds only a reference.
  ~scan_validation_arg() = default;

  result<void> validate(reader& scan_is) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->validate(scan_is);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> validate(reader& scan_is) const noexcept = 0;

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

    result<void> validate(reader& scan_is) const noexcept override {
      return validate_for<std::remove_cvref_t<T>>(scan_is);
    }

   protected:
    ~model_t() = default;
  };

  std::aligned_storage_t<sizeof(model_t<int>)> storage_;
};

/**
 * Type erased scan argument for scanning.
 */
class scan_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit scan_arg(T& value) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<T>*>(&storage_), value);
  }

  scan_arg(const scan_arg&) = delete;
  scan_arg(scan_arg&&) = delete;
  scan_arg& operator=(const scan_arg&) = delete;
  scan_arg& operator=(scan_arg&&) = delete;
  ~scan_arg() = default;  // No destructor & delete call to concept_t because model_t holds only a reference.

  result<void> parse_and_scan(reader& input, reader& scan_is) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->scan(input, scan_is);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> parse_and_scan(reader& input, reader& scan_is) const noexcept = 0;

   protected:
    ~concept_t() = default;
  };

  template <typename T>
  class model_t final : public concept_t {
   public:
    explicit model_t(T& value) noexcept : value_{value} {}

    model_t(const model_t&) = delete;
    model_t(model_t&&) = delete;
    model_t& operator=(const model_t&) = delete;
    model_t& operator=(model_t&&) = delete;

    result<void> parse_and_scan(reader& input, reader& scan_is) const noexcept override {
      scanner<std::remove_cvref_t<T>> scanner;
      EMIO_TRYV(invoke_scanner_parse<input_validation::disabled>(scanner, scan_is));
      return scanner.scan(input, value_);
    }

   protected:
    ~model_t() = default;

   private:
    T& value_;
  };

  std::aligned_storage_t<sizeof(model_t<std::string_view>)> storage_;
};

}  // namespace emio::detail::scan
