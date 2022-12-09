//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <array>
#include <bit>
#include <exception>
#include <limits>

namespace emio::detail {

/**
 * A constexpr bitset with the bare minimum implementation.
 * @tparam Bits The number of bits.
 */
template <size_t Bits>
class bitset {
 private:
  using word_t = size_t;
  static constexpr size_t bits_per_word = sizeof(word_t) * 8;
  static constexpr size_t number_of_words = (Bits / bits_per_word) + (((Bits % bits_per_word) == 0) ? 0 : 1);

 public:
  /**
   * Checks if all bits are set to true.
   * @return true if all bits are set to true, otherwise false
   */
  [[nodiscard]] constexpr bool all() const noexcept {
    if constexpr (Bits <= 0) {
      return true;
    } else {
      for (size_t i = 0; i < number_of_words - 1; i++) {
        if (words_[i] != ~word_t{0}) {
          return false;
        }
      }
      constexpr word_t high_word_mask = get_high_word_mask();
      return words_[number_of_words - 1] == high_word_mask;
    }
  }

  /**
   * Checks if the first n bits are set to true.
   * @param n - number of bits
   * @return true if the first n bits are set to true, otherwise false
   */
  [[nodiscard, gnu::noinline]] constexpr bool all_first(size_t n) const noexcept {
    // Prevent inlining because of a GCC compiler-bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=106921
    if constexpr (Bits <= 0) {
      return n == 0;
    } else {
      if (n > Bits) {
        return false;
      }
      size_t i = 0;
      for (; n > bits_per_word; n -= bits_per_word, i++) {
        if (words_[i] != ~word_t{0}) {  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index): ensured by loop
          return false;
        }
      }
      word_t last_word = words_[i];  // NOLINT(cppcoreguidelines-pro-bounds-constant-array-index): ensured by loop
      for (; n != 0; n--) {
        if ((last_word & 1) != 1) {
          return false;
        }
        last_word >>= 1;
      }
      return true;
    }
  }

  /**
   * Returns the number of bits that the bitset holds.
   * @return the number of bits
   */
  [[nodiscard]] constexpr size_t size() const noexcept {
    return Bits;
  }

  /**
   * Sets a specific bit to true.
   * @param pos - the position of the bit
   */
  constexpr void set(size_t pos) noexcept {
    if (pos >= Bits) {
      std::terminate();
    }
    // Get index of pos in words and truncate pos to word bits per word.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index): ensured by check before
    words_[pos / bits_per_word] |= word_t{1} << (pos % bits_per_word);
  }

 private:
  static constexpr word_t get_high_word_mask() noexcept {
    word_t high_word_mask = (word_t{1} << (Bits % (bits_per_word))) - word_t{1};
    if (high_word_mask == 0) {
      return std::numeric_limits<word_t>::max();
    }
    return high_word_mask;
  }

  std::array<word_t, number_of_words> words_{};
};

}  // namespace emio::detail
