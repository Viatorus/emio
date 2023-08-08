//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <cstdio>

#include "detail/format/format_to.hpp"
#include "iterator.hpp"

namespace emio {

/**
 * Provides access to the format string and the arguments to format.
 * @note This type should only be "constructed" via make_format_args(format_str, args...) and passed directly to a
 * formatting function.
 */
using format_args = detail::format::format_args;

// Alias template types.
template <typename... Args>
using format_string = detail::format::format_string<Args...>;

template <typename... Args>
using valid_format_string = detail::format::valid_format_string<Args...>;

/**
 * Returns an object that stores a format string with an array of all arguments to format.

 * @note The storage uses reference semantics and does not extend the lifetime of args. It is the programmer's
 * responsibility to ensure that args outlive the return value. Usually, the result is only used as argument to a
 * formatting function taking format_args by reference.

 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Internal type. Implicit convertible to format_args.
 */
template <typename... Args>
[[nodiscard]] detail::args_storage<detail::format::format_arg, sizeof...(Args)> make_format_args(
    format_string<Args...> format_str, const Args&... args) noexcept {
  return {format_str.get(), args...};
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param args The format args with the format string.
 * @return The total number of characters in the formatted string or invalid_format if the format string validation
 * failed.
 */
inline result<size_t> vformatted_size(format_args&& args) noexcept {
  detail::counting_buffer buf{};
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  return buf.count();
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param format_str The format string
 * @param args The arguments to be formatted.
 * @return The total number of characters in the formatted string.
 */
template <typename... Args>
[[nodiscard]] constexpr size_t formatted_size(valid_format_string<Args...> format_str,
                                              const Args&... args) noexcept(detail::exceptions_disabled) {
  detail::counting_buffer buf{};
  detail::format::format_to(buf, format_str, args...).value();
  return buf.count();
}

/**
 * Determines the total number of characters in the formatted string by formatting args according to the format string.
 * @param format_str The format string
 * @param args The arguments to be formatted.
 * @return The total number of characters in the formatted string on success or invalid_format if the format string
 * validation failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_string> || std::is_same_v<T, format_string<Args...>>)
constexpr result<size_t> formatted_size(T format_str, const Args&... args) noexcept {
  detail::counting_buffer buf{};
  format_string<Args...> str{format_str};
  EMIO_TRYV(detail::format::format_to(buf, str, args...));
  return buf.count();
}

/**
 * Formats arguments according to the format string, and writes the result to the output buffer.
 * @param buf The output buffer.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
template <typename Buffer>
  requires(std::is_base_of_v<buffer, Buffer>)
result<void> vformat_to(Buffer& buf, const format_args& args) noexcept {
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the writer's buffer.
 * @param wrt The writer.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
inline result<void> vformat_to(writer& wrt, const format_args& args) noexcept {
  EMIO_TRYV(detail::format::vformat_to(wrt.get_buffer(), args));
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the output iterator.
 * @param out The output iterator.
 * @param args The format args with the format string.
 * @return The iterator past the end of the output range on success or EOF if the buffer is to small or invalid_format
 * if the format string validation failed.
 */
template <typename OutputIt, typename... Args>
  requires(std::output_iterator<OutputIt, char>)
constexpr result<OutputIt> vformat_to(OutputIt out, const format_args& args) noexcept {
  iterator_buffer buf{out};
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  return buf.out();
}

/**
 * Formats arguments according to the format string, and writes the result to the output buffer.
 * @param buf The output buffer.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
template <typename Buffer, typename... Args>
  requires(std::is_base_of_v<buffer, Buffer>)
constexpr result<void> format_to(Buffer& buf, format_string<Args...> format_str, const Args&... args) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the writer's buffer.
 * @param wrt The writer.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return Success or EOF if the buffer is to small or invalid_format if the format string validation failed.
 */
template <typename... Args>
constexpr result<void> format_to(writer& wrt, format_string<Args...> format_str, const Args&... args) noexcept {
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(wrt.get_buffer(), format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(wrt.get_buffer(), make_format_args(format_str, args...)));
  }
  return success;
}

/**
 * Formats arguments according to the format string, and writes the result to the output iterator.
 * @param out The output iterator.
 * @param args The format args with the format string.
 * @return The iterator past the end of the output range on success or EOF if the buffer is to small or invalid_format
 * if the format string validation failed.
 */
template <typename OutputIt, typename... Args>
  requires(std::output_iterator<OutputIt, char>)
constexpr result<OutputIt> format_to(OutputIt out, format_string<Args...> format_str, const Args&... args) noexcept {
  iterator_buffer buf{out};
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  return buf.out();
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param args The format args with the format string.
 * @return The string on success or invalid_format if the format string validation
 * failed.
 */
inline result<std::string> vformat(const format_args& args) noexcept {
  memory_buffer buf;
  if (auto res = detail::format::vformat_to(buf, args); !res) {
    return res.assume_error();
  }
  return buf.str();
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The string.
 */
template <typename... Args>
[[nodiscard]] std::string format(valid_format_string<Args...> format_str,
                                 const Args&... args) noexcept(detail::exceptions_disabled) {
  return vformat(make_format_args(format_str, args...)).value();  // Should never fail.
}

/**
 * Formats arguments according to the format string, and returns the result as string.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The string on success or invalid_format if the format string validation
 * failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_string> || std::is_same_v<T, format_string<Args...>>)
result<std::string> format(T format_str, const Args&... args) noexcept {
  return vformat(make_format_args(format_str, args...));
}

/**
 * Return type of (v)format_to_n functions.
 * @tparam OutputIt The output iterator type.
 */
template <typename OutputIt>
struct format_to_n_result {
  OutputIt out;                           ///< The iterator past the end.
  std::iter_difference_t<OutputIt> size;  ///< The total number (not truncated) output size.
};

/**
 * Formats arguments according to the format string, and writes the result to the output iterator. At most n characters
 * are written.
 * @param out The output iterator.
 * @param n The maximum number of characters to be written to the buffer.
 * @param args The format args with the format string.
 * @return The format_to_n_result on success or invalid_format if the format string validation failed.
 */
template <typename OutputIt>
  requires(std::output_iterator<OutputIt, char>)
result<format_to_n_result<OutputIt>> vformat_to_n(OutputIt out, std::iter_difference_t<OutputIt> n,
                                                  const format_args& args) noexcept {
  truncating_iterator tout{out, static_cast<size_t>(n)};
  iterator_buffer buf{tout};
  EMIO_TRYV(detail::format::vformat_to(buf, args));
  tout = buf.out();
  return format_to_n_result<OutputIt>{tout.out(), tout.count()};
}

/**
 * Formats arguments according to the format string, and writes the result to the output iterator. At most n characters
 * are written.
 * @param out The output iterator.
 * @param n The maximum number of characters to be written to the buffer.
 * @param format_str The format string.
 * @param args The format args with the format string.
 * @return The format_to_n_result on success or invalid_format if the format string validation failed.
 */
template <typename OutputIt, typename... Args>
  requires(std::output_iterator<OutputIt, char>)
constexpr result<format_to_n_result<OutputIt>> format_to_n(OutputIt out, std::iter_difference_t<OutputIt> n,
                                                           format_string<Args...> format_str,
                                                           const Args&... args) noexcept {
  truncating_iterator tout{out, static_cast<size_t>(n)};
  iterator_buffer buf{tout};
  if (EMIO_Z_INTERNAL_IS_CONST_EVAL) {
    EMIO_TRYV(detail::format::format_to(buf, format_str, args...));
  } else {
    EMIO_TRYV(detail::format::vformat_to(buf, make_format_args(format_str, args...)));
  }
  tout = buf.out();
  return format_to_n_result<OutputIt>{tout.out(), tout.count()};
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream.
 * @param file The file stream.
 * @param args The format args with the format string.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
inline result<void> vprint(std::FILE* file, const format_args& args) noexcept {
  if (file == nullptr) {
    return err::invalid_data;
  }

  file_buffer buf{file};
  EMIO_TRYV(vformat_to(buf, args));
  return buf.flush();
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream.
 * @param format_str The format string.
 * @param args The format args with the format string.
 */
template <typename... Args>
void print(valid_format_string<Args...> format_str, const Args&... args) {
  vprint(stdout, make_format_args(format_str, args...)).value();  // Should never fail.
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_string> || std::is_same_v<T, format_string<Args...>>)
result<void> print(T format_str, const Args&... args) {
  return vprint(stdout, make_format_args(format_str, args...));
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream.
 * @param file The file stream.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename... Args>
result<void> print(std::FILE* file, format_string<Args...> format_str, const Args&... args) {
  return vprint(file, make_format_args(format_str, args...));
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream with a new line at the
 * end.
 * @param file The file stream.
 * @param args The format args with the format string.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
inline result<void> vprintln(std::FILE* file, const format_args& args) noexcept {
  if (file == nullptr) {
    return err::invalid_data;
  }

  file_buffer buf{file};
  EMIO_TRYV(vformat_to(buf, args));
  EMIO_TRY(auto area, buf.get_write_area_of(1));
  area[0] = '\n';
  return buf.flush();
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream with a new line
 * at the end.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 */
template <typename... Args>
void println(valid_format_string<Args...> format_str, const Args&... args) {
  vprintln(stdout, make_format_args(format_str, args...)).value();  // Should never fail.
}

/**
 * Formats arguments according to the format string, and writes the result to the standard output stream with a new line
 * at the end.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename T, typename... Args>
  requires(std::is_same_v<T, runtime_string> || std::is_same_v<T, format_string<Args...>>)
result<void> println(T format_str, const Args&... args) {
  return vprintln(stdout, make_format_args(format_str, args...));
}

/**
 * Formats arguments according to the format string, and writes the result to a file stream with a new line
 * at the end.
 * @param file The file stream.
 * @param format_str The format string.
 * @param args The arguments to be formatted.
 * @return Success or EOF if the file stream is not writable or invalid_format if the format string validation failed.
 */
template <typename... Args>
result<void> println(std::FILE* file, format_string<Args...> format_str, const Args&... args) {
  return vprintln(file, make_format_args(format_str, args...));
}

}  // namespace emio
