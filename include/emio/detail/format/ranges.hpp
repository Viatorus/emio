//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "../../formatter.hpp"

namespace emio::detail::format {

// For range.

using std::begin;
using std::data;
using std::end;
using std::size;

template <typename T>
concept advanceable = requires(T x) { ++x; };

template <typename T>
concept is_iterable = std::is_array_v<T> || requires(T x) {
  { begin(x) } -> advanceable;
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

template <typename T>
struct is_span : std::false_type {};

template <typename T, size_t N>
struct is_span<std::span<T, N>> : std::true_type {};

template <typename T>
concept is_contiguous_but_not_span = std::is_array_v<T> || requires(T x) {
  requires !is_span<T>::value;
  requires std::is_same_v<std::remove_cvref_t<decltype(*data(x))>, element_type_t<T>>;
  { size(x) } -> std::same_as<size_t>;
};

struct ranges_specs {
  std::string_view opening_bracket{};
  std::string_view closing_bracket{};
  std::string_view separator{};
};

template <typename Formatter>
  requires requires(Formatter f) { f.set_debug_format(true); }
constexpr void maybe_set_debug_format(Formatter& f, bool set) noexcept {
  f.set_debug_format(set);
}

template <typename Formatter>
constexpr void maybe_set_debug_format(Formatter& /*unused*/, ...) noexcept {}

// For tuple like types.

using std::get;

// From https://stackoverflow.com/a/68444475/1611317
template <class T, std::size_t N>
concept has_tuple_element = requires(T t) {
  typename std::tuple_element_t<N, std::remove_const_t<T>>;
  { get<N>(t) } -> std::convertible_to<const std::tuple_element_t<N, T>&>;
};

template <typename T, size_t... Ns>
constexpr auto has_tuple_element_unpack(std::index_sequence<Ns...> /*unused*/) noexcept {
  return (has_tuple_element<T, Ns> && ...);
}

template <class T>
concept is_tuple_like = !std::is_reference_v<T> && requires(T t) {
  typename std::tuple_size<T>::type;
  requires std::derived_from<std::tuple_size<T>, std::integral_constant<std::size_t, std::tuple_size_v<T>>>;
} && has_tuple_element_unpack<T>(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T, size_t... Ns>
constexpr auto is_formattable_unpack(std::index_sequence<Ns...> /*unused*/) noexcept {
  return (is_formattable_v<decltype(get<Ns>(std::declval<T&>()))> && ...);
}

template <typename T>
concept is_valid_tuple = !is_valid_range<T> && is_tuple_like<T> &&
                         is_formattable_unpack<T>(std::make_index_sequence<std::tuple_size_v<T>>());

template <typename T, std::size_t... Ns>
auto get_tuple_formatters(std::index_sequence<Ns...> /*unused*/)
    -> std::tuple<formatter<std::remove_cvref_t<std::tuple_element_t<Ns, T>>>...>;

template <typename T>
using tuple_formatters = decltype(get_tuple_formatters<T>(std::make_index_sequence<std::tuple_size_v<T>>{}));

}  // namespace emio::detail::format
