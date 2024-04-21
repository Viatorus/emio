//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <algorithm>
#include <cstring>
#include <string_view>
#include <type_traits>

#include "detail/conversion.hpp"
#include "result.hpp"

namespace emio {

// Forward declaration.
class reader;

namespace detail {

inline constexpr const char*& get_it(reader& rdr) noexcept;

inline constexpr const char* get_end(reader& rdr) noexcept;

inline constexpr result<bool> parse_sign(reader& in) noexcept;

template <typename T>
constexpr result<T> parse_int(reader& in, int base, bool is_negative) noexcept;

}  // namespace detail

/**
 * This class operates on a char sequence and allows reading and parsing from it.
 * The reader interprets the char sequence as finite input stream. After every successful operation the read position
 * moves on until the last char of the sequence has been consumed.
 */
class reader {
 public:
  /// The size type.
  using size_type = typename std::string_view::size_type;
  /// Special value to indicate the end of the view or an error by functions returning an index.
  static constexpr size_type npos = std::string_view::npos;

  /**
   * Constructs an empty reader.
   */
  constexpr reader() = default;

  // Don't allow temporary strings or any nullptr.
  constexpr reader(std::nullptr_t) = delete;
  constexpr reader(int) = delete;
#if __STDC_HOSTED__
  constexpr reader(std::string&&) = delete;  // NOLINT(cppcoreguidelines-rvalue-reference-param-not-moved): as intended
#endif

  /**
   * Constructs the reader from any suitable char sequence.
   * @param input The char sequence.
   */
  template <typename Arg>
    requires(std::is_constructible_v<std::string_view, Arg> && !std::is_same_v<Arg, std::string_view>)
  // NOLINTNEXTLINE(bugprone-forwarding-reference-overload): Is guarded by require clause.
  constexpr explicit reader(Arg&& input) noexcept : reader{std::string_view{std::forward<Arg>(input)}} {}

  /**
   * Constructs the reader from a string view.
   * @param sv The string view.
   */
  constexpr explicit reader(const std::string_view& sv) noexcept : begin_{sv.begin()}, it_{begin_}, end_{sv.end()} {}

  /**
   * Returns the current read position.
   * @return The read position.
   */
  [[nodiscard]] constexpr size_t pos() const noexcept {
    return static_cast<size_t>(it_ - begin_);
  }

  /**
   * Checks if the end of the stream has been reached.
   * @return True if reached and all chars are read, otherwise false.
   */
  [[nodiscard]] constexpr bool eof() const noexcept {
    return end_ == it_;
  }

  /**
   * Returns the number of the not yet read chars of the stream.
   * @return The number of remaining chars.
   */
  [[nodiscard]] constexpr size_t cnt_remaining() const noexcept {
    return static_cast<size_t>(end_ - it_);
  }

  /**
   * Obtains a view of the not yet read chars of the stream.
   * @return The view over the remaining chars.
   */
  [[nodiscard]] constexpr std::string_view view_remaining() const noexcept {
    return {it_, end_};
  }

  /**
   * Reads all remaining chars from the stream.
   * @return The remaining chars. Could be empty.
   */
  [[nodiscard]] constexpr std::string_view read_remaining() noexcept {
    const std::string_view remaining_view = view_remaining();
    it_ = end_;
    return remaining_view;
  }

  /**
   * Pops one (default) or n chars from the reader.
   * @note Does never overflow.
   * @param cnt The number of chars to pop.
   */
  constexpr void pop(const size_t cnt = 1) noexcept {
    if (static_cast<size_t>(end_ - it_) >= cnt) {
      it_ += cnt;
    } else {
      it_ = end_;
    }
  }

  /**
   * Makes one (default) or n chars available again to read.
   * @note Does never underflow.
   * @param cnt The number of chars to unpop.
   */
  constexpr void unpop(const size_t cnt = 1) noexcept {
    if (static_cast<size_t>(it_ - begin_) >= cnt) {
      it_ -= cnt;
    } else {
      it_ = begin_;
    }
  }

  /**
   * Returns a newly constructed reader over the not yet read char sequence of the range [pos, pos + len).
   * If len is greater than the size of the remaining chars, the end of the char sequence is used.
   * @param pos The position of the first char to include.
   * @param len The length of the char sequence.
   * @return EOF if the position is outside the char sequence.
   */
  constexpr result<reader> subreader(const size_t pos, const size_t len = npos) const noexcept {
    const char* const next_it = it_ + pos;
    if (next_it > end_) {
      return err::eof;
    }
    const size_t rlen = std::min(len, static_cast<size_t>(end_ - next_it));
    return reader{std::string_view{next_it, rlen}};
  }

  /**
   * Returns the next char from the stream without consuming it.
   * @return EOF if the end of the stream has been reached.
   */
  constexpr result<char> peek() const noexcept {
    if (it_ != end_) {
      return *it_;
    }
    return err::eof;
  }

  /**
   * Reads one char from the stream.
   * @return EOF if the end of the stream has been reached.
   */
  constexpr result<char> read_char() noexcept {
    if (it_ != end_) {
      return *it_++;
    }
    return err::eof;
  }

  /**
   * Reads n chars from the stream.
   * @param n The number of chars to read.
   * @return EOF if the end of the stream has been reached before reading n chars.
   */
  constexpr result<std::string_view> read_n_chars(const size_t n) noexcept {
    if (static_cast<size_t>(end_ - it_) >= n) {
      std::string_view res{it_, it_ + n};
      it_ += n;
      return res;
    }
    return err::eof;
  }

  /**
   * Parses an integer from the stream.
   * @tparam T The type of the integer.
   * @param base The input base of the integer. Must be greater equal 2 and less equal 36.
   * @return invalid_argument if the requested input base is not supported, EOF if the end of the stream has been
   * reached or invalid_data if the char sequence cannot be parsed as integer.
   */
  template <typename T>
    requires(std::is_integral_v<T>)
  constexpr result<T> parse_int(const int base = 10) noexcept {
    // Store current read position.
    const char* const backup_it = it_;

    // Reduce code generation by upcasting the integer.
    using upcasted_t = detail::upcasted_int_t<T>;
    const result<upcasted_t> res = parse_sign_and_int<upcasted_t>(base);
    if (!res) {
      it_ = backup_it;
      return res.assume_error();
    }
    if constexpr (std::is_same_v<upcasted_t, T>) {
      return res;
    } else {
      // Check if upcast int is within the integer type range.
      const upcasted_t val = res.assume_value();
      if (val < std::numeric_limits<T>::min() || val > std::numeric_limits<T>::max()) {
        it_ = backup_it;
        return err::out_of_range;
      }
      return static_cast<T>(val);
    }
  }

  /**
   * Parse options for read_until operations.
   */
  struct read_until_options {
    bool include_delimiter{false};  ///< If true, the delimiter is part of the output char sequence, otherwise not.
    bool keep_delimiter{false};     ///< If true, the delimiter will be kept inside the stream, otherwise consumed.
    bool ignore_eof{false};  ///< If true, reaching EOF does result in a failed read, otherwise in a successfully read.
  };

  /**
   * Reads multiple chars from the stream until a given char as delimiter is reached or EOF (configurable).
   * @param delimiter The char delimiter.
   * @param options The read until options.
   * @return invalid_data if the delimiter hasn't been found and ignore_eof is set to true or EOF if the stream is
   * empty.
   */
  constexpr result<std::string_view> read_until_char(
      const char delimiter, const read_until_options& options = default_read_until_options()) noexcept {
    return read_until_match(std::find(it_, end_, delimiter), options);
  }

  /**
   * Reads multiple chars from the stream until a given char sequence as delimiter is reached or EOF (configurable).
   * @param delimiter The char sequence.
   * @param options The read until options.
   * @return invalid_data if the delimiter hasn't been found and ignore_eof is set to true or EOF if the stream is
   * empty.
   */
  constexpr result<std::string_view> read_until_str(const std::string_view& delimiter,
                                                    const read_until_options& options = default_read_until_options()) {
    return read_until_pos(view_remaining().find(delimiter), options, delimiter.size());
  }

  /**
   * Reads multiple chars from the stream until a char of a given group is reached or EOF (configurable).
   * @param group The char group.
   * @param options The read until options.
   * @return invalid_data if no char has been found and ignore_eof is set to True or EOF if the stream is empty.
   */
  constexpr result<std::string_view> read_until_any_of(
      const std::string_view& group, const read_until_options& options = default_read_until_options()) noexcept {
    return read_until_pos(view_remaining().find_first_of(group), options);
  }

  /**
   * Reads multiple chars from the stream until no char of a given group is reached or EOF (configurable).
   * @param group The char group.
   * @param options The read until options.
   * @return invalid_data if a char not in the group has been found and ignore_eof is set to True or EOF if the stream
   * is empty.
   */
  constexpr result<std::string_view> read_until_none_of(
      const std::string_view& group, const read_until_options& options = default_read_until_options()) noexcept {
    return read_until_pos(view_remaining().find_first_not_of(group), options);
  }

  /**
   * Reads multiple chars from the stream until a given predicate returns true or EOF is reached (configurable).
   * @param predicate The predicate taking a char and returning a bool.
   * @param options The read until options.
   * @return invalid_data if the predicate never returns true and ignore_eof is set to True or EOF if the stream is
   * empty.
   */
  template <typename Predicate>
    requires(std::is_invocable_r_v<bool, Predicate, char>)
  constexpr result<std::string_view>
  read_until(const Predicate& predicate, const read_until_options& options = default_read_until_options()) noexcept(
      std::is_nothrow_invocable_r_v<bool, Predicate, char>) {
    return read_until_match(std::find_if(it_, end_, predicate), options);
  }

  /**
   * Reads one char from the stream if the char matches the expected one.
   * @param c The expected char.
   * @return invalid_data if the chars don't match or EOF if the end of the stream has been reached.
   */
  constexpr result<char> read_if_match_char(const char c) noexcept {
    if (it_ != end_) {
      if (*it_ == c) {
        ++it_;
        return c;
      }
      return err::invalid_data;
    }
    return err::eof;
  }

  /**
   * Reads multiple chars from the stream if the chars matches the expected char sequence.
   * @param sv The expected char sequence.
   * @return invalid_data if the chars don't match or EOF if the end of the stream has been reached.
   */
  constexpr result<std::string_view> read_if_match_str(const std::string_view& sv) noexcept {
    const size_t n = sv.size();
    if (static_cast<size_t>(end_ - it_) < n) {
      return err::eof;
    }
    if (detail::equal_n(it_, sv.begin(), n)) {
      const std::string_view res{it_, n};
      it_ += n;
      return res;
    }
    return err::invalid_data;
  }

 private:
  friend constexpr const char*& detail::get_it(reader& rdr) noexcept;
  friend constexpr const char* detail::get_end(reader& rdr) noexcept;

  // Helper function since GCC and Clang complain about "member initializer for '...' needed within definition of
  // enclosing class". Which is a bug.
  [[nodiscard]] static constexpr read_until_options default_read_until_options() noexcept {
    return {};
  }

  template <typename T>
  constexpr result<T> parse_sign_and_int(const int base) noexcept {
    EMIO_TRY(const bool is_negative, detail::parse_sign(*this));
    return detail::parse_int<T>(*this, base, is_negative);
  }

  constexpr result<std::string_view> read_until_pos(size_t pos, const read_until_options& options,
                                                    const size_type delimiter_size = 1) noexcept {
    const char* match = end_;
    if (pos != npos) {
      match = it_ + pos;
    }
    return read_until_match(match, options, delimiter_size);
  }

  constexpr result<std::string_view> read_until_match(const char* match, const read_until_options& options,
                                                      const size_type delimiter_size = 1) noexcept {
    if (it_ == end_) {
      return err::eof;
    }
    const char* const begin = it_;
    if (match == end_) {
      if (!options.ignore_eof) {
        it_ = end_;
        return std::string_view{begin, end_};
      }
      return err::invalid_data;
    }
    it_ = match;
    if (!options.keep_delimiter) {
      it_ += delimiter_size;
    }
    if (options.include_delimiter) {
      match += delimiter_size;
    }
    return std::string_view{begin, match};
  }

  const char* begin_{};
  const char* it_{};
  const char* end_{};
};

namespace detail {

inline constexpr const char*& get_it(reader& rdr) noexcept {
  return rdr.it_;
}

inline constexpr const char* get_end(reader& rdr) noexcept {
  return rdr.end_;
}

inline constexpr result<bool> parse_sign(reader& in) noexcept {
  bool is_negative = false;
  EMIO_TRY(const char c, in.peek());
  if (c == '+') {
    in.pop();
  } else if (c == '-') {
    is_negative = true;
    in.pop();
  }
  return is_negative;
}

template <typename T>
constexpr result<T> parse_int(reader& in, const int base, const bool is_negative) noexcept {
  if (!is_valid_number_base(base)) {
    return err::invalid_argument;
  }

  EMIO_TRY(const char c, in.read_char());
  std::optional<int> digit = char_to_digit(c, base);
  if (!digit) {
    return err::invalid_data;
  }

  if constexpr (std::is_unsigned_v<T>) {
    if (is_negative) {
      return err::out_of_range;
    }
  }

  T value{};
  T maybe_overflowed_value{};
  const auto has_next_digit = [&]() noexcept {
    value = maybe_overflowed_value;

    const result<char> res = in.peek();
    if (!res) {
      return false;
    }
    digit = detail::char_to_digit(res.value(), base);
    if (!digit) {
      return false;
    }
    in.pop();
    maybe_overflowed_value = value * static_cast<T>(base);
    return true;
  };

  if constexpr (std::is_signed_v<T>) {
    if (is_negative) {
      while (true) {
        maybe_overflowed_value -= static_cast<T>(*digit);
        if (maybe_overflowed_value > value) {
          return err::out_of_range;
        }
        if (!has_next_digit()) {
          return value;
        }
      }
    }
  }
  while (true) {
    maybe_overflowed_value += static_cast<T>(*digit);
    if (maybe_overflowed_value < value) {
      return err::out_of_range;
    }
    if (!has_next_digit()) {
      return value;
    }
  }
}

}  // namespace detail

}  // namespace emio
