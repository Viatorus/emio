//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>
#include <cstddef>
#include <string>

#include "conversion.hpp"
#include "predef.hpp"

namespace emio::detail {

/**
 * A constexpr basic_string with the bare minimum implementation.
 * @tparam Char The character type.
 */
template <typename Char>
class ct_basic_string {
 public:
  constexpr ct_basic_string() = default;

  ct_basic_string(const ct_basic_string&) = delete;
  ct_basic_string(ct_basic_string&&) = delete;
  ct_basic_string& operator=(const ct_basic_string&) = delete;
  ct_basic_string& operator=(ct_basic_string&&) = delete;

  constexpr ~ct_basic_string() noexcept {
    delete[] data_;  // NOLINT(cppcoreguidelines-owning-memory)
  }

  constexpr void resize(size_t new_size) noexcept {
    // Heavy pointer arithmetic because high level containers are not yet ready to use at constant evaluation.
    if (data_ == nullptr) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      data_ = new Char[new_size]{};  // NOLINT(cppcoreguidelines-owning-memory)
      capacity_ = new_size;
    } else if (capacity_ < new_size) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      Char* new_data = new Char[new_size]{};  // NOLINT(cppcoreguidelines-owning-memory)
      copy_n(data_, size_, new_data);         // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      std::swap(new_data, data_);
      capacity_ = new_size;
      delete[] new_data;  // NOLINT(cppcoreguidelines-owning-memory)
    } else if (size_ < new_size) {
      fill_n(data_ + size_, new_size - size_, Char{});  // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    }
    size_ = new_size;
  }

  [[nodiscard]] constexpr size_t capacity() const noexcept {
    return capacity_;
  }

  [[nodiscard]] constexpr size_t size() const noexcept {
    return size_;
  }

  [[nodiscard]] constexpr Char* data() noexcept {
    return data_;
  }

  [[nodiscard]] constexpr const Char* data() const noexcept {
    return data_;
  }

 private:
  Char* data_{};
  size_t size_{};
  size_t capacity_{};
};

/**
 * This union unites std::basic_string for runtime and ct_basic_string for compile-time.
 * @tparam Char The character type.
 */
template <typename Char>
union basic_string_union {
  constexpr basic_string_union() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      std::construct_at(&ct_str_);
    } else {
      std::construct_at(&str_);
    }
  }

  constexpr basic_string_union(const basic_string_union&) = delete;
  constexpr basic_string_union(basic_string_union&&) noexcept = delete;
  constexpr basic_string_union& operator=(const basic_string_union&) = delete;
  constexpr basic_string_union& operator=(basic_string_union&&) noexcept = default;

  constexpr ~basic_string_union() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      ct_str_.~ct_basic_string();
    } else {
      str_.~basic_string();
    }
  }

  [[nodiscard]] constexpr size_t capacity() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.capacity();
    } else {
      return str_.capacity();
    }
  }

  constexpr void resize(size_t size) noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      ct_str_.resize(size);
    } else {
      str_.resize(size);
    }
  }

  constexpr Char* data() noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.data();
    } else {
      return str_.data();
    }
  }

  [[nodiscard]] constexpr const Char* data() const noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.data();
    } else {
      return str_.data();
    }
  }

  [[nodiscard]] constexpr size_t size() const noexcept {
    if (Y_EMIO_IS_CONST_EVAL) {
      return ct_str_.size();
    } else {
      return str_.size();
    }
  }

 private:
  std::basic_string<Char> str_;
  detail::ct_basic_string<Char> ct_str_;
};

}  // namespace emio::detail
