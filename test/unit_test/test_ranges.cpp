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
#include <emio/ranges.hpp>

// Other includes.
#include <fmt/ranges.h>

#include <catch2/catch_test_macros.hpp>
#include <emio/format.hpp>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <vector>

// Test cases from fmt/test/a-test.cc - 9.1.0

TEST_CASE("format_array", "[ranges]") {
  const int arr[] = {1, 2, 3, 5, 7, 11};
  CHECK(emio::format("{}", arr) == "[1, 2, 3, 5, 7, 11]");
}

TEST_CASE("ranges invalid_format or validate tests", "[ranges]") {
  const int arr[] = {1, 2};
  CHECK(emio::format(emio::runtime{"{}"}, arr) == "[1, 2]");
  CHECK(emio::format(emio::runtime{"{:x"}, arr) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime{"{:n"}, arr) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime{"{::i}"}, arr) == emio::err::invalid_format);

  std::array<char, 1> out;
  emio::span_buffer buf{out};
  CHECK(emio::format_to(buf, "{}", arr) == emio::err::eof);
  CHECK(emio::format_to(buf, "x{}", arr) == emio::err::eof);
}

TEST_CASE("format_2d_array", "[ranges]") {
  int arr[][2] = {{1, 2}, {3, 5}, {7, 11}};
  CHECK(emio::format("{}", arr) == "[[1, 2], [3, 5], [7, 11]]");
}

TEST_CASE("format_array_of_literals", "[ranges]") {
  const char* arr[] = {"1234", "abcd"};
  CHECK(emio::format("{}", arr) == "[\"1234\", \"abcd\"]");
  CHECK(emio::format("{:n}", arr) == "\"1234\", \"abcd\"");
  CHECK(emio::format("{:n:}", arr) == "1234, abcd");
}

TEST_CASE("format_vector", "[ranges]") {
  auto v = std::vector<int>{1, 2, 3, 5, 7, 11};
  CHECK(emio::format("{}", v) == "[1, 2, 3, 5, 7, 11]");
  CHECK(emio::format("{::#x}", v) == "[0x1, 0x2, 0x3, 0x5, 0x7, 0xb]");
  CHECK(emio::format("{:n:#x}", v) == "0x1, 0x2, 0x3, 0x5, 0x7, 0xb");
}

TEST_CASE("format_vector2", "[ranges]") {
  auto v = std::vector<std::vector<int>>{{1, 2}, {3, 5}, {7, 11}};
  CHECK(emio::format("{}", v) == "[[1, 2], [3, 5], [7, 11]]");
  CHECK(emio::format("{:::#x}", v) == "[[0x1, 0x2], [0x3, 0x5], [0x7, 0xb]]");
  CHECK(emio::format("{:n:n:#x}", v) == "0x1, 0x2, 0x3, 0x5, 0x7, 0xb");
}

TEST_CASE("format_map", "[ranges]") {
  auto m = std::map<std::string, int>{{"one", 1}, {"two", 2}};
  CHECK(emio::format("{}", m) == "{\"one\": 1, \"two\": 2}");
  CHECK(emio::format("{:n}", m) == "\"one\": 1, \"two\": 2");
  CHECK(emio::format(emio::runtime{"{:::}"}, m).value() == "{one: 1, two: 2}");
  CHECK(emio::format(emio::runtime{"{:::^7}"}, m).value() == "{  one  :    1   ,   two  :    2   }");
}

TEST_CASE("format_set", "[ranges]") {
  CHECK(emio::format("{}", std::set<std::string>{"one", "two"}) == "{\"one\", \"two\"}");
}

namespace adl {
struct box {
  int value;
};

auto begin(const box& b) -> const int* {
  return &b.value;
}

auto end(const box& b) -> const int* {
  return &b.value + 1;
}
}  // namespace adl

TEST_CASE("format_adl_begin_end", "[ranges]") {
  auto b = adl::box{42};
  CHECK(emio::format("{}", b) == "[42]");
}

TEST_CASE("format_pair", "[ranges]") {
  auto p = std::pair<int, float>(42, 1.5f);
  CHECK(emio::format("{}", p) == "(42, 1.5)");
  CHECK(emio::format("{:n}", p) == "42, 1.5");
  CHECK(emio::format("{::<5}", p) == "(42   , 1.5  )");
}

TEST_CASE("tuple_like invalid_format or validate tests", "[ranges]") {
  auto p = std::pair<int, float>(42, 1.5f);
  CHECK(emio::format(emio::runtime{"{}"}, p) == "(42, 1.5)");
  CHECK(emio::format(emio::runtime{"{:x"}, p) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime{"{:n"}, p) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime{"{::i}"}, p) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime{"{::}"}, std::tuple<>()) == emio::err::invalid_format);

  std::array<char, 1> out;
  emio::span_buffer buf{out};
  CHECK(emio::format_to(buf, "{}", p) == emio::err::eof);
  CHECK(emio::format_to(buf, "x{}", p) == emio::err::eof);
}

struct unformattable {};

TEST_CASE("format_tuple", "[ranges]") {
  auto t = std::tuple<int, float, std::string, char>(42, 1.5f, "this is tuple", 'i');
  CHECK(emio::format("{}", t) == "(42, 1.5, \"this is tuple\", 'i')");
  CHECK(emio::format("{}", std::tuple<>()) == "()");
  CHECK(emio::format("{:n}", std::tuple<>()) == "");

  STATIC_CHECK(emio::is_formattable_v<std::tuple<>>);
  STATIC_CHECK(!emio::is_formattable_v<unformattable>);
  STATIC_CHECK(!emio::is_formattable_v<std::tuple<unformattable>>);
  STATIC_CHECK(!emio::is_formattable_v<std::tuple<unformattable, int>>);
  STATIC_CHECK(!emio::is_formattable_v<std::tuple<int, unformattable>>);
  STATIC_CHECK(!emio::is_formattable_v<std::tuple<unformattable, unformattable>>);
  STATIC_CHECK(emio::is_formattable_v<std::tuple<int, float>>);
}

struct tuple_like {
  int i;
  std::string str;

  template <size_t N>
    requires(N == 0)
  int get() const noexcept {
    return i;
  }
  template <size_t N>
    requires(N == 1)
  std::string_view get() const noexcept {
    return str;
  }
};

template <size_t N>
auto get(const tuple_like& t) noexcept -> decltype(t.get<N>()) {
  return t.get<N>();
}

namespace std {
template <>
struct tuple_size<tuple_like> : std::integral_constant<size_t, 2> {};

template <size_t N>
struct tuple_element<N, tuple_like> {
  using type = decltype(std::declval<tuple_like>().get<N>());
};
}  // namespace std

TEST_CASE("format_struct", "[ranges]") {
  auto t = tuple_like{42, "foo"};
  CHECK(emio::format("{}", t) == "(42, \"foo\")");
}

TEST_CASE("format_to", "[ranges]") {
  char buf[10];
  auto end = emio::format_to(buf, "{}", std::vector<int>{1, 2, 3}).value();
  *end = '\0';
  CHECK(std::string_view{buf} == "[1, 2, 3]");
}

// struct path_like {
//   const path_like* begin() const;
//   const path_like* end() const;
//
//   operator std::string() const;
// };
//
// TEST_CASE("path_like", "[ranges]") {
//   EXPECT_FALSE((fmt::is_range<path_like, char>::value));
// }
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
//   STATIC_CHECK((!fmt::has_formatter<std::vector<unformattable>, fmt::format_context>::value));
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

TEST_CASE("escape_string", "[ranges]") {
  using vec = std::vector<std::string>;
  CHECK(emio::format("{}", vec{"\n\r\t\"\\"}) == "[\"\\n\\r\\t\\\"\\\\\"]");
  CHECK(emio::format("{}", vec{"\x07"}) == "[\"\\x07\"]");
  CHECK(emio::format("{}", vec{"\x7f"}) == "[\"\\x7f\"]");
  CHECK(emio::format("{}", vec{"n\xcc\x83"}) == "[\"n\\xcc\\x83\"]");

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
}

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
