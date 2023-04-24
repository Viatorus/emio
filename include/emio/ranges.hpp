//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include "formatter.hpp"

#include <tuple>
#include <utility>

namespace emio {

template <typename Arg>
inline constexpr bool is_formattable_v = detail::format::has_formatter_v<std::remove_cvref_t<Arg>>;

namespace detail {

template <typename T>
concept Advanceable = requires(T x) { ++x; };

using std::begin;
using std::end;

template <typename T>
concept is_iterable = std::is_array_v<T> || requires(T x) {
                                              { begin(x) } -> Advanceable;
                                              requires !std::is_same_v<decltype(*begin(x)), void>;
                                              { static_cast<bool>(begin(x) != end(x)) };
                                            };

template <typename T>
using element_type_t = std::remove_cvref_t<decltype(*begin(std::declval<std::remove_reference_t<T>&>()))>;

template <typename T>
concept is_map = requires(T x) { typename T::mapped_type; };

template <typename T>
concept is_set = requires(T x) { typename T::key_type; };

template <typename T>
concept is_string_like = std::is_constructible_v<std::string_view, T>;

template <typename T>
concept is_valid_range = is_iterable<T> && !is_string_like<T> && is_formattable_v<element_type_t<T>>;

}  // namespace detail
namespace detail::format {

struct ranges_specs {
  std::string_view opening_bracket{};
  std::string_view closing_bracket{};
  std::string_view separator{};
};

}  // namespace detail::format

// Set: "{" "}" ", "
// Map: "{" "}" ": " ", "
// pair/tuple: "(" ")" ", "

template <typename T>
  requires(detail::is_valid_range<T>)
class formatter<T> {
 public:
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
    EMIO_TRY(char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    return success;
  }

  constexpr formatter() noexcept
    requires(!detail::is_map<T> && !detail::is_set<T>)
      : specs_{"[", "]", ", "} {}

  constexpr formatter() noexcept
    requires(detail::is_map<T>)
      : specs_{"{", "}", ", "} {
    set_brackets("{", "}");
    underlying_.set_separator(": ");
  }

  constexpr formatter() noexcept
    requires(detail::is_set<T>)
      : specs_{"{", "}", ", "} {
    underlying_.set_separator(": ");
  }

  constexpr void set_separator(std::string_view separator) noexcept {
    specs_.separator = separator;
  }

  constexpr void set_brackets(std::string_view opening_bracket, std::string_view closing_bracket) noexcept {
    specs_.opening_bracket = opening_bracket;
    specs_.closing_bracket = closing_bracket;
  }

  constexpr result<void> parse(reader<char>& rdr) noexcept {
    EMIO_TRY(char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    return success;
  }

  constexpr result<void> format(writer<char>& wtr, const T& arg) noexcept {
    EMIO_TRYV(wtr.write_str(specs_.opening_bracket));

    using std::begin;
    using std::end;
    auto first = begin(arg);
    const auto last = end(arg);
    for (auto it = first; it != last; ++it) {
      if (it != first) {
        EMIO_TRYV(wtr.write_str(specs_.separator));
      }
      EMIO_TRYV(underlying_.format(wtr, *it));
    }
    EMIO_TRYV(wtr.write_str(specs_.closing_bracket));
    return success;
  }

  constexpr auto& underlying() noexcept {
    return underlying_;
  }

 private:
  formatter<detail::element_type_t<T>> underlying_{};
  detail::format::ranges_specs specs_{};
};

namespace detail {

// From https://stackoverflow.com/a/68444475/1611317
template <class T, std::size_t N>
concept has_tuple_element = requires(T t) {
                              typename std::tuple_element_t<N, std::remove_const_t<T>>;
                              { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
                            };

template <class T>
concept is_tuple_like =
    !std::is_reference_v<T> &&
    requires(T t) {
      typename std::tuple_size<T>::type;
      requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
    } && []<std::size_t... N>(std::index_sequence<N...>) {
      return (has_tuple_element<T, N> && ...);
    }(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T>
concept is_valid_tuple = !is_valid_range<T> && is_tuple_like<T> && []<std::size_t... N>(std::index_sequence<N...>) {
  return (is_formattable_v<decltype(std::get<N>(std::declval<T&>()))> && ...);
}(std::make_index_sequence<std::tuple_size_v<T>>());

// template <typename T>
// using tuple_formatters = typename decltype([]<std::size_t... N>(std::index_sequence<N...>) {
//   return (std::type_identity<std::tuple<formatter<std::remove_cvref_t<std::tuple_element_t<N, T>>>...>>{});
// }(std::make_index_sequence<std::tuple_size_v<T>>()))::type;

template <typename T, std::size_t... Ns>
auto get_tuple_formatters(std::index_sequence<Ns...>)
    -> std::tuple<formatter<std::remove_cvref_t<std::tuple_element_t<Ns, T>>>...>;

template <typename T>
using tuple_formatters = decltype(get_tuple_formatters<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

}  // namespace detail

template <typename T>
  requires(detail::is_valid_tuple<T>)
class formatter<T> {
 public:
  constexpr formatter() : specs_{"(", ")", ", "} {}

  constexpr void set_separator(std::string_view separator) noexcept {
    specs_.separator = separator;
  }

  constexpr void set_brackets(std::string_view opening_bracket, std::string_view closing_bracket) noexcept {
    specs_.opening_bracket = opening_bracket;
    specs_.closing_bracket = closing_bracket;
  }

  constexpr result<void> parse(reader<char>& rdr) noexcept {
    EMIO_TRY(char c, rdr.read_char());
    if (c != '}') {  // No format end.
      return err::invalid_format;
    }
    return success;
  }

  constexpr result<void> format(writer<char>& wtr, const T& args) noexcept {
    EMIO_TRYV(wtr.write_str(specs_.opening_bracket));
    EMIO_TRYV(format_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), wtr, args));
    EMIO_TRYV(wtr.write_str(specs_.closing_bracket));
    return success;
  }

 private:
  template <size_t... Ns>
  [[nodiscard]] constexpr result<void> format_for_each(std::index_sequence<Ns...>, writer<char>& wtr,
                                                       const T& args) noexcept {
    size_t i = 0;
    result<void> res = success;
    const auto form = [&](auto& f, const auto& arg) {
      if (i++ != 0) {
        res = wtr.write_str(specs_.separator);
        if (res.has_error()) {
          return false;
        }
      }
      res = f.format(wtr, arg);
      return res.has_value();
    };
    (form(std::get<Ns>(formatters_), std::get<Ns>(args)) && ...);
    return res;
  }

  detail::tuple_formatters<T> formatters_{};
  detail::format::ranges_specs specs_{};
};

} // namespace emio
