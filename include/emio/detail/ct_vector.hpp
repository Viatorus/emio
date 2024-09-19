//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

#include "conversion.hpp"
#include "predef.hpp"

namespace emio::detail {

/**
 * A constexpr vector with the bare minimum implementation and inlined storage.
 * @tparam Char The character type.
 * @tparam StorageSize The size of the inlined storage.
 */
template <typename Char, size_t StorageSize = 128>
class ct_vector {
 public:
  constexpr ct_vector() noexcept {
    if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
      fill_n(storage_.data(), storage_.size(), 0);
    }
  }

  constexpr ct_vector(const ct_vector& other) : ct_vector() {
    reserve(other.size_);
    copy_n(other.data_, other.size_, data_);
  }

  constexpr ct_vector(ct_vector&& other) noexcept : ct_vector() {
    // Transfer ownership.
    if (other.hold_external()) {
      data_ = other.data_;
      capacity_ = other.capacity_;
    } else {
      copy_n(other.data_, other.size_, data_);
    }
    size_ = other.size_;

    // Reset other.
    other.data_ = other.storage_.data();
    other.size_ = 0;
    other.capacity_ = StorageSize;
  }

  constexpr ct_vector& operator=(const ct_vector& other) {
    reserve(other.size_);
    copy_n(other.data_, other.size_, data_);
    return *this;
  }

  constexpr ct_vector& operator=(ct_vector&& other) noexcept {
    // Free this.
    if (hold_external()) {
      delete[] data_;  // NOLINT(cppcoreguidelines-owning-memory)
    }

    // Transfer ownership.
    if (other.hold_external()) {
      data_ = other.data_;
      capacity_ = other.capacity_;
    } else {
      copy_n(other.data_, other.size_, data_);
    }
    size_ = other.size_;

    // Reset other.
    other.data_ = other.storage_.data();
    other.size_ = 0;
    other.capacity_ = StorageSize;

    return *this;
  }

  constexpr ~ct_vector() noexcept {
    if (hold_external()) {
      delete[] data_;  // NOLINT(cppcoreguidelines-owning-memory)
    }
  }

  constexpr void reserve(size_t new_size) noexcept {
    if (new_size < StorageSize && !hold_external()) {
      size_ = new_size;
      return;
    }

    // Heavy pointer arithmetic because high level containers are not yet ready to use at constant evaluation.
    if (capacity_ < new_size) {
      // NOLINTNEXTLINE(bugprone-unhandled-exception-at-new): char types cannot throw
      Char* new_data = new Char[new_size];  // NOLINT(cppcoreguidelines-owning-memory)
      copy_n(data_, size_, new_data);
      if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
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

  constexpr void clear() noexcept {
    size_ = 0;
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

  std::array<Char, StorageSize> storage_;
  Char* data_{storage_.data()};
  size_t size_{};
  size_t capacity_{StorageSize};
};

}  // namespace emio::detail
