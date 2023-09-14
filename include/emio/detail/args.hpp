//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

namespace emio::detail {

/**
 * Type erased argument to validate.
 */
template <template <typename> typename Trait>
class validation_arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit validation_arg(std::type_identity<T> /*unused*/) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<typename Trait<T>::unified_type>*>(&storage_));
  }

  validation_arg(const validation_arg&) = delete;
  validation_arg(validation_arg&&) = delete;
  validation_arg& operator=(const validation_arg&) = delete;
  validation_arg& operator=(validation_arg&&) = delete;
  // No destructor & delete call to concept_t because model_t holds only a reference.
  ~validation_arg() = default;

  result<void> validate(reader& format_rdr) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->validate(format_rdr);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> validate(reader& format_rdr) const noexcept = 0;

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

    result<void> validate(reader& format_rdr) const noexcept override {
      return Trait<std::remove_cvref_t<T>>::validate(format_rdr);
    }

   protected:
    ~model_t() = default;
  };

  std::aligned_storage_t<sizeof(model_t<int>)> storage_;
};

/**
 * Type erased argument to parse and process.
 */
template <typename Input, template <typename> typename Trait>
class arg {
 public:
  template <typename T>
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): will be initialized in constructor
  explicit arg(T& value) noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to use the storage
    std::construct_at(reinterpret_cast<model_t<typename Trait<T>::unified_type>*>(&storage_), value);
  }

  arg(const arg&) = delete;
  arg(arg&&) = delete;
  arg& operator=(const arg&) = delete;
  arg& operator=(arg&&) = delete;
  ~arg() = default;  // No destructor & delete call to concept_t because model_t holds only a reference.

  result<void> process_arg(Input& input, reader& format_rdr) const noexcept {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast): only way to get the object back
    return reinterpret_cast<const concept_t*>(&storage_)->process_arg(input, format_rdr);
  }

 private:
  class concept_t {
   public:
    concept_t() = default;
    concept_t(const concept_t&) = delete;
    concept_t(concept_t&&) = delete;
    concept_t& operator=(const concept_t&) = delete;
    concept_t& operator=(concept_t&&) = delete;

    virtual result<void> process_arg(Input& input, reader& format_rdr) const noexcept = 0;

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

    result<void> process_arg(Input& input, reader& format_rdr) const noexcept override {
      return Trait<std::remove_cvref_t<T>>::process_arg(input, format_rdr, value_);
    }

   protected:
    ~model_t() = default;

   private:
    T value_;
  };

  std::aligned_storage_t<sizeof(model_t<std::string_view>)> storage_;
};

/**
 * Format arguments just for format string validation.
 */
template <typename T>
class args_span {
 public:
  args_span(const args_span&) = delete;
  args_span(args_span&&) = delete;
  args_span& operator=(const args_span&) = delete;
  args_span& operator=(args_span&&) = delete;
  ~args_span() = default;

  [[nodiscard]] result<std::string_view> get_str() const noexcept {
    return format_str_;
  }

  [[nodiscard]] std::span<const T> get_args() const noexcept {
    return args_;
  }

 protected:
  args_span(result<std::string_view> format_str, std::span<const T> args) : format_str_{format_str}, args_{args} {}

 private:
  result<std::string_view> format_str_;
  std::span<const T> args_;
};

/**
 * Format arguments storage just for format string validation.
 */
template <typename Arg, size_t NbrOfArgs>
class args_storage : public args_span<Arg> {
 public:
  template <typename... Args>
  args_storage(result<std::string_view> str, Args&&... args) noexcept
      : args_span<Arg>{str, args_storage_}, args_storage_{Arg{std::forward<Args>(args)}...} {}

  args_storage(const args_storage&) = delete;
  args_storage(args_storage&&) = delete;
  args_storage& operator=(const args_storage&) = delete;
  args_storage& operator=(args_storage&&) = delete;
  ~args_storage() = default;

 private:
  std::array<Arg, NbrOfArgs> args_storage_;
};

template <typename T, typename... Args>
args_storage<T, sizeof...(Args)> make_validation_args(std::string_view format_str) noexcept {
  return {format_str, std::type_identity<Args>{}...};
}

}  // namespace emio::detail
