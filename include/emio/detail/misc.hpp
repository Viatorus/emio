//
// Copyright (c) 20213- present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <type_traits>

namespace emio::detail {

template <typename T>
struct always_false : std::false_type {};

template <typename T>
inline constexpr bool always_false_v = always_false<T>::value;

template <typename... Ts>
struct overload : Ts... {
  using Ts::operator()...;
};

template <class... Ts>
overload(Ts...) -> overload<Ts...>;

}  // namespace emio::detail
