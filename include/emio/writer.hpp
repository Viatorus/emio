//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>

#include "buffer.hpp"
#include "detail/conversion.hpp"
#include "detail/utf.hpp"

namespace emio {

/**
 * This class operates on a buffer and allows writing sequences of characters or other kinds of data into it.
 */
class writer {
 public:
  /**
   * Constructs a writer with a given buffer.
   * @param buf The buffer.
   */
  constexpr writer(buffer& buf) noexcept : buf_{buf} {}

  /**
   * Returns the buffer.
   * @return The buffer.
   */
  [[nodiscard]] constexpr buffer& get_buffer() noexcept {
    return buf_;
  }

  /**
   * Writes a character into the buffer.
   * @param c The character.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char(const char c) noexcept {
    EMIO_TRY(const auto area, buf_.get_write_area_of(1));
    area[0] = c;
    return success;
  }

  /**
   * Writes a character n times into the buffer.
   * @param c The character.
   * @param n The number of times the character should be written.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char_n(const char c, const size_t n) noexcept {
    // Perform write in multiple chunks, to support buffers with an internal cache.
    size_t remaining_size = n;
    while (remaining_size != 0) {
      EMIO_TRY(const auto area, buf_.get_write_area_of_max(remaining_size));
      detail::fill_n(area.data(), area.size(), c);
      remaining_size -= area.size();
    }
    return success;
  }

  /**
   * Writes a character escaped into the buffer.
   * @param c The character.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_char_escaped(const char c) noexcept {
    const std::string_view sv(&c, 1);
    return detail::write_str_escaped(buf_, sv, detail::count_size_when_escaped(sv), '\'');
  }

  /**
   * Writes a char sequence into the buffer.
   * @param sv The char sequence.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_str(const std::string_view sv) noexcept {
    // Perform write in multiple chunks, to support buffers with an internal cache.
    const char* ptr = sv.data();
    size_t remaining_size = sv.size();
    while (remaining_size != 0) {
      EMIO_TRY(const auto area, buf_.get_write_area_of_max(remaining_size));
      detail::copy_n(ptr, area.size(), area.data());
      remaining_size -= area.size();
      ptr += area.size();
    }
    return success;
  }

  /**
   * Writes a char sequence escaped into the buffer.
   * @param sv The char sequence.
   * @return EOF if the buffer is to small.
   */
  constexpr result<void> write_str_escaped(const std::string_view sv) noexcept {
    return detail::write_str_escaped(buf_, sv, detail::count_size_when_escaped(sv), '"');
  }

  /**
   * Format options for writing integers.
   */
  struct write_int_options {
    int base{10};            ///< The output base of the integer. Must be greater equal 2 and less equal 36.
    bool upper_case{false};  ///< If true, the letters are upper case, otherwise lower case.
  };

  /**
   * Writes an integer into the buffer.
   * @param integer The integer.
   * @param options The integer options.
   * @return invalid_argument if the requested output base is not supported or EOF if the buffer is to small.
   */
  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<void> write_int(const T integer,
                                   const write_int_options& options = default_write_int_options()) noexcept {
    // Reduce code generation by upcasting the integer.
    return write_int_impl(detail::integer_upcast(integer), options);
  }

 private:
  // Helper function since GCC and Clang complain about "member initializer for '...' needed within definition of
  // enclosing class". Which is a bug.
  static constexpr write_int_options default_write_int_options() noexcept {
    return {};
  }

  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<void> write_int_impl(const T integer, const write_int_options& options) noexcept {
    if (!detail::is_valid_number_base(options.base)) {
      return err::invalid_argument;
    }
    const auto abs_number = detail::to_absolute(integer);
    const bool negative = detail::is_negative(integer);
    const size_t number_of_digits =
        detail::get_number_of_digits(abs_number, options.base) + static_cast<size_t>(negative);

    EMIO_TRY(const auto area, buf_.get_write_area_of(number_of_digits));
    if (negative) {
      area[0] = '-';
    }
    detail::write_number(abs_number, options.base, options.upper_case,
                         area.data() + detail::to_signed(number_of_digits));
    return success;
  }

  buffer& buf_;
};

}  // namespace emio
