// Formatting library for C++ - the core API
//
// Copyright (c) 2012 - present, Victor Zverovich
// All rights reserved.
//
// For the license information refer to format.h.
//
// Copyright (c) 2018 - present, Remotion (Igor Schulz)
// All Rights Reserved
// {fmt} support for ranges, containers and types tuple interface.

// Unit under test.
#include <emio/format.hpp>

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
using element_type_t = std::decay_t<decltype(*begin(std::declval<std::remove_reference_t<T>&>()))>;

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

}  // namespace emio

// Other includes.
#include <fmt/ranges.h>

#include <catch2/catch_test_macros.hpp>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

// Test cases from fmt/test/a-test.cc - 9.1.0

TEST_CASE("format_array", "[ranges]") {
  int arr[] = {1, 2, 3, 5, 7, 11};
  //  CHECK(emio::format("{}", arr) == "[1, 2, 3, 5, 7, 11]");

  static_assert(!emio::detail::is_valid_tuple<std::array<int, 4>>);
  static_assert(emio::detail::is_valid_tuple<std::tuple<int, float>>);
  static_assert(emio::detail::is_valid_tuple<std::pair<int, float>>);
  static_assert(!emio::detail::is_valid_tuple<std::pair<int, float*>>);

  std::tuple<int, int, int> tup{1, 2, 3};
  CHECK(emio::format("{}", tup) == "(1, 2, 3)");

  //  emio::detail::tuple_formatters<std::tuple<int, int, int>> x = 1;

  using T = const decltype(tup);
  static_assert(emio::detail::is_valid_tuple<T>);
  //  static_assert(emio::detail::is_valid_range<std::remove_cvref_t<T>>);
  //  static_assert(emio::is_formattable_v<T>);
  //  static_assert(emio::is_formattable_v<std::remove_cvref_t<T>>);

  //  emio::formatter<int[6]> s;

  //  auto m = std::map<std::string, int>{{"one", 1}, {"two", 2}};
  //  auto s = std::set<std::string>{"one", "two"};
  //  CHECK(emio::format("{}", m) == "{\"one\": 1, \"two\": 2}");
  //  CHECK(emio::format("{}", s) == "{\"one\": 1, \"two\": 2}");
}

// TEST_CASE("format_2d_array", "[ranges]") {
//   int arr[][2] = {{1, 2}, {3, 5}, {7, 11}};
//   CHECK(emio::format("{}", arr) == "[[1, 2], [3, 5], [7, 11]]");
// }
//
// TEST_CASE("format_array_of_literals", "[ranges]") {
//   const char* arr[] = {"1234", "abcd"};
//   CHECK(emio::format("{}", arr) == "[\"1234\", \"abcd\"]");
//   CHECK(emio::format("{:n}", arr) == "\"1234\", \"abcd\"");
//   CHECK(emio::format("{:n:}", arr) == "1234, abcd");
// }
// #endif  // FMT_RANGES_TEST_ENABLE_C_STYLE_ARRAY
//
// TEST_CASE("format_vector", "[ranges]") {
//   auto v = std::vector<int>{1, 2, 3, 5, 7, 11};
//   CHECK(emio::format("{}", v) == "[1, 2, 3, 5, 7, 11]");
//   CHECK(emio::format("{::#x}", v) == "[0x1, 0x2, 0x3, 0x5, 0x7, 0xb]");
//   CHECK(emio::format("{:n:#x}", v) == "0x1, 0x2, 0x3, 0x5, 0x7, 0xb");
// }
//
// TEST_CASE("format_vector2", "[ranges]") {
//   auto v = std::vector<std::vector<int>>{{1, 2}, {3, 5}, {7, 11}};
//   CHECK(emio::format("{}", v) == "[[1, 2], [3, 5], [7, 11]]");
//   CHECK(emio::format("{:::#x}", v) == "[[0x1, 0x2], [0x3, 0x5], [0x7, 0xb]]");
//   CHECK(emio::format("{:n:n:#x}", v) == "0x1, 0x2, 0x3, 0x5, 0x7, 0xb");
// }
//
// TEST_CASE("format_map", "[ranges]") {
//   auto m = std::map<std::string, int>{{"one", 1}, {"two", 2}};
//   CHECK(emio::format("{}", m) == "{\"one\": 1, \"two\": 2}");
//   CHECK(emio::format("{:n}", m) == "\"one\": 1, \"two\": 2");
// }
//
// TEST_CASE("format_set", "[ranges]") {
//   CHECK(emio::format("{}", std::set<std::string>{"one", "two"}) == "{\"one\", \"two\"}");
// }
//
// namespace adl {
// struct box {
//   int value;
// };
//
// auto begin(const box& b) -> const int* {
//   return &b.value;
// }
//
// auto end(const box& b) -> const int* {
//   return &b.value + 1;
// }
// }  // namespace adl
//
// TEST_CASE("format_adl_begin_end", "[ranges]") {
//   auto b = adl::box{42};
//   CHECK(emio::format("{}", b) == "[42]");
// }
//
// TEST_CASE("format_pair", "[ranges]") {
//   auto p = std::pair<int, float>(42, 1.5f);
//   CHECK(emio::format("{}", p) == "(42, 1.5)");
// }
//
// struct unformattable {};
//
// TEST_CASE("format_tuple", "[ranges]") {
//   auto t = std::tuple<int, float, std::string, char>(42, 1.5f, "this is tuple", 'i');
//   CHECK(emio::format("{}", t) == "(42, 1.5, \"this is tuple\", 'i')");
//   CHECK(emio::format("{}", std::tuple<>()) == "()");
//
//   //  EXPECT_TRUE((fmt::is_formattable<std::tuple<>>::value));
//   //  EXPECT_FALSE((fmt::is_formattable<unformattable>::value));
//   //  EXPECT_FALSE((fmt::is_formattable<std::tuple<unformattable>>::value));
//   //  EXPECT_FALSE((fmt::is_formattable<std::tuple<unformattable, int>>::value));
//   //  EXPECT_FALSE((fmt::is_formattable<std::tuple<int, unformattable>>::value));
//   //  EXPECT_FALSE((fmt::is_formattable<std::tuple<unformattable, unformattable>>::value));
//   //  EXPECT_TRUE((fmt::is_formattable<std::tuple<int, float>>::value));
// }
//
// struct tuple_like {
//   int i;
//   std::string str;
//
//   template <size_t N>
//   fmt::enable_if_t<N == 0, int> get() const noexcept {
//     return i;
//   }
//   template <size_t N>
//   fmt::enable_if_t<N == 1, fmt::string_view> get() const noexcept {
//     return str;
//   }
// };
//
// template <size_t N>
// auto get(const tuple_like& t) noexcept -> decltype(t.get<N>()) {
//   return t.get<N>();
// }
//
// namespace std {
// template <>
// struct tuple_size<tuple_like> : std::integral_constant<size_t, 2> {};
//
// template <size_t N>
// struct tuple_element<N, tuple_like> {
//   using type = decltype(std::declval<tuple_like>().get<N>());
// };
// }  // namespace std
//
// TEST_CASE("format_struct", "[ranges]") {
//   auto t = tuple_like{42, "foo"};
//   CHECK(emio::format("{}", t) == "(42, \"foo\")");
// }
//
// TEST_CASE("format_to", "[ranges]") {
//   char buf[10];
//   auto end = fmt::format_to(buf, "{}", std::vector<int>{1, 2, 3});
//   *end = '\0';
//   EXPECT_STREQ(buf, "[1, 2, 3]");
// }
//
//// struct path_like {
////   const path_like* begin() const;
////   const path_like* end() const;
////
////   operator std::string() const;
//// };
////
//// TEST_CASE("path_like", "[ranges]") {
////   EXPECT_FALSE((fmt::is_range<path_like, char>::value));
//// }
//
//// A range that provides non-const only begin()/end() to test fmt::join handles
//// that.
////
//// Some ranges (e.g. those produced by range-v3's views::filter()) can cache
//// information during iteration so they only provide non-const begin()/end().
// template <typename T>
// class non_const_only_range {
//  private:
//   std::vector<T> vec;
//
//  public:
//   using const_iterator = typename ::std::vector<T>::const_iterator;
//
//   template <typename... Args>
//   explicit non_const_only_range(Args&&... args) : vec(std::forward<Args>(args)...) {}
//
//   const_iterator begin() {
//     return vec.begin();
//   }
//   const_iterator end() {
//     return vec.end();
//   }
// };
//
// template <typename T>
// class noncopyable_range {
//  private:
//   std::vector<T> vec;
//
//  public:
//   using const_iterator = typename ::std::vector<T>::const_iterator;
//
//   template <typename... Args>
//   explicit noncopyable_range(Args&&... args) : vec(std::forward<Args>(args)...) {}
//
//   noncopyable_range(noncopyable_range const&) = delete;
//   noncopyable_range(noncopyable_range&) = delete;
//
//   const_iterator begin() const {
//     return vec.begin();
//   }
//   const_iterator end() const {
//     return vec.end();
//   }
// };
//
// TEST_CASE("range", "[ranges]") {
//   noncopyable_range<int> w(3u, 0);
//   CHECK(emio::format("{}", w) == "[0, 0, 0]");
//   CHECK(emio::format("{}", noncopyable_range<int>(3u, 0)) == "[0, 0, 0]");
//
//   non_const_only_range<int> x(3u, 0);
//   CHECK(emio::format("{}", x) == "[0, 0, 0]");
//   CHECK(emio::format("{}", non_const_only_range<int>(3u, 0)) == "[0, 0, 0]");
//
//   auto y = std::vector<int>(3u, 0);
//   CHECK(emio::format("{}", y) == "[0, 0, 0]");
//   CHECK(emio::format("{}", std::vector<int>(3u, 0)) == "[0, 0, 0]");
//
//   const auto z = std::vector<int>(3u, 0);
//   CHECK(emio::format("{}", z) == "[0, 0, 0]");
// }
//
// enum test_enum { foo };
// auto format_as(test_enum e) -> int {
//   return e;
// }
//
// TEST_CASE("enum_range", "[ranges]") {
//   auto v = std::vector<test_enum>{test_enum::foo};
//   CHECK(emio::format("{}", v) == "[0]");
// }
//
// TEST_CASE("unformattable_range", "[ranges]") {
//   //  EXPECT_FALSE((fmt::has_formatter<std::vector<unformattable>, fmt::format_context>::value));
// }
//
// #ifdef FMT_RANGES_TEST_ENABLE_JOIN
// TEST_CASE("join_tuple", "[ranges]") {
//   // Value tuple args.
//   auto t1 = std::tuple<char, int, float>('a', 1, 2.0f);
//   CHECK(emio::format("({})", fmt::join(t1, ", ")) == "(a, 1, 2)");
//
//   // Testing lvalue tuple args.
//   int x = 4;
//   auto t2 = std::tuple<char, int&>('b', x);
//   CHECK(emio::format("{}", fmt::join(t2, " + ")) == "b + 4");
//
//   // Empty tuple.
//   auto t3 = std::tuple<>();
//   CHECK(emio::format("{}", fmt::join(t3, "|")) == "");
//
//   // Single element tuple.
//   auto t4 = std::tuple<float>(4.0f);
//   CHECK(emio::format("{}", fmt::join(t4, "/")) == "4");
//
// #  if FMT_TUPLE_JOIN_SPECIFIERS
//   // Specs applied to each element.
//   auto t5 = std::tuple<int, int, long>(-3, 100, 1);
//   CHECK(emio::format("{:+03}", fmt::join(t5, ", ")) == "-03, +100, +01");
//
//   auto t6 = std::tuple<float, double, long double>(3, 3.14, 3.1415);
//   CHECK(emio::format("{:5.5f}", fmt::join(t6, ", ")) == "3.00000, 3.14000, 3.14150");
//
//   // Testing lvalue tuple args.
//   int y = -1;
//   auto t7 = std::tuple<int, int&, const int&>(3, y, y);
//   CHECK(emio::format("{:03}", fmt::join(t7, ", ")) == "003, -01, -01");
// #  endif
// }
//
// TEST_CASE("join_initializer_list", "[ranges]") {
//   CHECK(emio::format("{}", fmt::join({1, 2, 3}, ", ")) == "1, 2, 3");
//   CHECK(emio::format("{}", fmt::join({"fmt", "rocks", "!"}, " ")) == "fmt rocks !");
// }
//
// struct zstring_sentinel {};
//
// bool operator==(const char* p, zstring_sentinel) {
//   return *p == '\0';
// }
// bool operator!=(const char* p, zstring_sentinel) {
//   return *p != '\0';
// }
//
// struct zstring {
//   const char* p;
//   const char* begin() const {
//     return p;
//   }
//   zstring_sentinel end() const {
//     return {};
//   }
// };
//
// #  ifdef __cpp_lib_ranges
// struct cpp20_only_range {
//   struct iterator {
//     int val = 0;
//
//     using value_type = int;
//     using difference_type = std::ptrdiff_t;
//     using iterator_concept = std::input_iterator_tag;
//
//     iterator() = default;
//     iterator(int i) : val(i) {}
//     int operator*() const {
//       return val;
//     }
//     iterator& operator++() {
//       ++val;
//       return *this;
//     }
//     void operator++(int) {
//       ++*this;
//     }
//     bool operator==(const iterator& rhs) const {
//       return val == rhs.val;
//     }
//   };
//
//   int lo;
//   int hi;
//
//   iterator begin() const {
//     return iterator(lo);
//   }
//   iterator end() const {
//     return iterator(hi);
//   }
// };
//
// static_assert(std::input_iterator<cpp20_only_range::iterator>);
// #  endif
//
// TEST_CASE("join_sentinel", "[ranges]") {
//   auto hello = zstring{"hello"};
//   CHECK(emio::format("{}", hello) == "['h', 'e', 'l', 'l', 'o']");
//   CHECK(emio::format("{::}", hello) == "[h, e, l, l, o]");
//   CHECK(emio::format("{}", fmt::join(hello, "_")) == "h_e_l_l_o");
// }
//
// TEST_CASE("join_range", "[ranges]") {
//   noncopyable_range<int> w(3u, 0);
//   CHECK(emio::format("{}", fmt::join(w, ",")) == "0,0,0");
//   CHECK(emio::format("{}", fmt::join(noncopyable_range<int>(3u, 0) == ",")), "0,0,0");
//
//   non_const_only_range<int> x(3u, 0);
//   CHECK(emio::format("{}", fmt::join(x, ",")) == "0,0,0");
//   CHECK(emio::format("{}", fmt::join(non_const_only_range<int>(3u, 0) == ",")), "0,0,0");
//
//   auto y = std::vector<int>(3u, 0);
//   CHECK(emio::format("{}", fmt::join(y, ",")) == "0,0,0");
//   CHECK(emio::format("{}", fmt::join(std::vector<int>(3u, 0) == ",")), "0,0,0");
//
//   const auto z = std::vector<int>(3u, 0);
//   CHECK(emio::format("{}", fmt::join(z, ",")) == "0,0,0");
//
// #  ifdef __cpp_lib_ranges
//   CHECK(emio::format("{}", cpp20_only_range{.lo = 0, .hi = 5}) == "[0, 1, 2, 3, 4]");
//   CHECK(emio::format("{}", fmt::join(cpp20_only_range{.lo = 0, .hi = 5}, ",")) == "0,1,2,3,4");
// #  endif
// }
// #endif  // FMT_RANGES_TEST_ENABLE_JOIN
//
// TEST_CASE("escape_string", "[ranges]") {
//   using vec = std::vector<std::string>;
//   CHECK(emio::format("{}", vec{"\n\r\t\"\\"}) == "[\"\\n\\r\\t\\\"\\\\\"]");
//   CHECK(emio::format("{}", vec{"\x07"}) == "[\"\\x07\"]");
//   CHECK(emio::format("{}", vec{"\x7f"}) == "[\"\\x7f\"]");
//   CHECK(emio::format("{}", vec{"n\xcc\x83"}) == "[\"n\xcc\x83\"]");
//
//   if (fmt::detail::is_utf8()) {
//     CHECK(emio::format("{}", vec{"\xcd\xb8"}) == "[\"\\u0378\"]");
//     // Unassigned Unicode code points.
//     CHECK(emio::format("{}", vec{"\xf0\xaa\x9b\x9e"}) == "[\"\\U0002a6de\"]");
//     // Broken utf-8.
//     CHECK(emio::format("{}", vec{"\xf4\x8f\xbf\xc0"}) == "[\"\\xf4\\x8f\\xbf\\xc0\"]");
//     CHECK(emio::format("{}", vec{"\xf0\x28"}) == "[\"\\xf0(\"]");
//     CHECK(emio::format("{}", vec{"\xe1\x28"}) == "[\"\\xe1(\"]");
//     CHECK(emio::format("{}", vec{std::string("\xf0\x28\0\0anything", 12)}) == "[\"\\xf0(\\x00\\x00anything\"]");
//
//     // Correct utf-8.
//     CHECK(emio::format("{}", vec{"понедельник"}) == "[\"понедельник\"]");
//   }
// }
//
// template <typename R>
// struct fmt_ref_view {
//   R* r;
//
//   auto begin() const -> decltype(r->begin()) {
//     return r->begin();
//   }
//   auto end() const -> decltype(r->end()) {
//     return r->end();
//   }
// };
//
// TEST_CASE("range_of_range_of_mixed_const", "[ranges]") {
//   std::vector<std::vector<int>> v = {{1, 2, 3}, {4, 5}};
//   CHECK(emio::format("{}", v) == "[[1, 2, 3], [4, 5]]");
//
//   fmt_ref_view<decltype(v)> r{&v};
//   CHECK(emio::format("{}", r) == "[[1, 2, 3], [4, 5]]");
// }
//
// TEST_CASE("vector_char", "[ranges]") {
//   CHECK(emio::format("{}", std::vector<char>{'a', 'b'}) == "['a', 'b']");
// }
