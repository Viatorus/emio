//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>

#include "conversion.hpp"
#include "predef.hpp"

namespace emio::detail {

/**
 * A constexpr basic_string with the bare minimum implementation.
 * @tparam Char The character type.
 */
template <typename Char, size_t storage_size = 32>
class ct_basic_string {
 public:
  constexpr ct_basic_string() {
    if (Y_EMIO_IS_CONST_EVAL) {
      fill_n(storage_.data(), storage_.size(), 0);
    }
  }

  ct_basic_string(const ct_basic_string&) = delete;
  ct_basic_string(ct_basic_string&&) = delete;
  ct_basic_string& operator=(const ct_basic_string&) = delete;
  ct_basic_string& operator=(ct_basic_string&&) = delete;

  constexpr ~ct_basic_string() noexcept {
    if (hold_external()) {
      delete[] data_;  // NOLINT(cppcoreguidelines-owning-memory)
    }
  }

  constexpr void reserve(size_t new_size) noexcept {
    if (new_size < storage_size && !hold_external()) {
      size_ = new_size;
      return;
    }

    // Heavy pointer arithmetic because high level containers are not yet ready to use at constant evaluation.
    if (capacity_ < new_size) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      Char* new_data = new Char[new_size];  // NOLINT(cppcoreguidelines-owning-memory)
      copy_n(data_, size_, new_data);       // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
      if (Y_EMIO_IS_CONST_EVAL) {
        // Required at compile-time because another reserve could happen without previous write to the data.
        fill_n(new_data + size_, new_size - size_, 0);
      }
      std::swap(new_data, data_);
      capacity_ = new_size;
      if (new_data != storage_.data()) {
        delete[] new_data;  // NOLINT(cppcoreguidelines-owning-memory)
      }
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
  [[nodiscard]] constexpr bool hold_external() const noexcept {
    return data_ != storage_.data() && data_ != nullptr;
  }

  std::array<char, storage_size> storage_;
  Char* data_{storage_.data()};
  size_t size_{};
  size_t capacity_{storage_size};
};

}  // namespace emio::detail
