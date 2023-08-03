//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "../formatter.hpp"
#include "../scanner.hpp"

namespace emio::detail {

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
  args_storage(result<std::string_view> str, Args&&... args)
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
args_storage<T, sizeof...(Args)> make_validation_args(std::string_view format_str) {
  return {format_str, std::type_identity<Args>{}...};
}

}  // namespace emio::detail