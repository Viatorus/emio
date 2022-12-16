// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

namespace {

struct wrap {
  int id;
};

}  // namespace

template <>
class emio::formatter<wrap> : public emio::formatter<int> {
 public:
  template <typename Char>
  [[nodiscard]] constexpr result<void> format(writer<Char>& wtr, const wrap& arg) noexcept {
    return formatter<int>::format(wtr, arg.id);
  }
};

TEST_CASE("formatter with inherit from emio::formatter", "[formatter]") {
  // Test strategy:
  // * Format a custom type with a formatter which inherit from a base emio::formatter.
  // Expected: The formatting works and the format options from the base emio::formatter can be used.

  SECTION("compile-time") {
    SECTION("simple") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        static_cast<void>(emio::format_to(buf, "{}", wrap{42}).value());
        return buf.view() == "42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("complex") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        static_cast<void>(emio::format_to(buf, "{:x<4x}", wrap{42}).value());
        return buf.view() == "2axx";
      }();
      STATIC_CHECK(success);
    }
  }

  CHECK(emio::format("{}", wrap{42}) == "42");
  CHECK(emio::format("{:x<4x}", wrap{42}) == "2axx");
}

namespace {

struct foo {
  int id;
};

}  // namespace

template <>
class emio::formatter<foo> {
 public:
  template <typename Char>
  [[nodiscard]] static constexpr result<void> validate(reader<Char>& rdr) noexcept {
    EMIO_TRY(Char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      return rdr.read_if_match_char('}');
    }
    return emio::err::invalid_format;
  }

  template <typename Char>
  [[nodiscard]] constexpr result<void> parse(reader<Char>& rdr) noexcept {
    Char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      sign_ = -1;
    }
    rdr.pop();
    return success;
  }

  template <typename Char>
  [[nodiscard]] constexpr result<void> format(writer<Char>& wtr, const foo& arg) noexcept {
    return emio::format_to(wtr.get_buffer(), "foo: {}", sign_ * arg.id);
  }

 private:
  int sign_{1};
};

TEST_CASE("formatter with constexpr methods", "[formatter]") {
  // Test strategy:
  // * Format a custom type with an own formatter providing all necessary constexpr methods.
  // Expected: The formatting and the format options work.

  SECTION("compile-time") {
    SECTION("simple") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        static_cast<void>(emio::format_to(buf, "{}", foo{42}).value());
        return buf.view() == "foo: 42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("advanced") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        static_cast<void>(emio::format_to(buf, "{:-}", foo{42}).value());
        return buf.view() == "foo: -42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("error") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        return emio::format_to(buf, emio::runtime{"{:+}"}, foo{42}) == emio::err::invalid_format;
      }();
      STATIC_CHECK(success);
    }
  }

  CHECK(emio::format("{}", foo{42}) == "foo: 42");
  CHECK(emio::format("{:-}", foo{42}) == "foo: -42");
  CHECK(emio::format(emio::runtime{"{:+}"}, foo{42}) == emio::err::invalid_format);
}

namespace {

struct bar {};

}  // namespace

template <>
class emio::formatter<bar> {
 public:
  template <typename Char>
  [[nodiscard]] static result<void> validate(reader<Char>& rdr) noexcept {
    EMIO_TRY(Char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      return rdr.read_if_match_char('}');
    }
    return emio::err::invalid_format;
  }

  template <typename Char>
  [[nodiscard]] result<void> parse(reader<Char>& rdr) noexcept {
    Char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      upper_case_ = true;
    }
    rdr.pop();
    return success;
  }

  template <typename Char>
  [[nodiscard]] result<void> format(writer<Char>& wtr, const bar& /*arg*/) noexcept {
    if (upper_case_) {
      return wtr.write_str("BAR");
    }
    return wtr.write_str("bar");
  }

 private:
  bool upper_case_{false};
};

TEST_CASE("formatter with non-constexpr methods", "[formatter]") {
  // Test strategy:
  // * Format a custom type with an own formatter providing all necessary non-constexpr methods.
  // Expected: The formatting works.

  CHECK(emio::format(emio::runtime{"{}"}, bar{}) == "bar");
  CHECK(emio::format(emio::runtime{"{:-}"}, bar{}) == "BAR");
  CHECK(emio::format(emio::runtime{"{:+}"}, bar{}) == emio::err::invalid_format);
}

namespace {

struct foobar {
  int id;
};

}  // namespace

template <>
class emio::formatter<foobar> {
 public:
  template <typename Char>
  [[nodiscard]] constexpr result<void> parse(reader<Char>& rdr) noexcept {
    EMIO_TRY(Char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      sign_ = -1;
      return rdr.read_if_match_char('}');
    }
    return emio::err::invalid_format;
  }

  template <typename Char>
  [[nodiscard]] constexpr result<void> format(writer<Char>& wtr, const foobar& arg) noexcept {
    return emio::format_to(wtr.get_buffer(), "foobar: {}", sign_ * arg.id);
  }

 private:
  int sign_{1};
};

TEST_CASE("formatter with constexpr methods but without validate function", "[formatter]") {
  // Test strategy:
  // * Format a custom type with an own formatter providing all necessary constexpr methods.
  // Expected: The formatting and the format options work.

  SECTION("compile-time") {
    SECTION("simple") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        static_cast<void>(emio::format_to(buf, "{}", foobar{42}).value());
        return buf.view() == "foobar: 42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("advanced") {
      constexpr bool success = [] {
        std::array<char, 11> arr;
        emio::span_buffer buf{arr};
        static_cast<void>(emio::format_to(buf, "{:-}", foobar{42}).value());
        return buf.view() == "foobar: -42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("error") {
      constexpr bool success = [] {
        std::array<char, 10> arr;
        emio::span_buffer buf{arr};
        return emio::format_to(buf, emio::runtime{"{:+}"}, foobar{42}) == emio::err::invalid_format;
      }();
      STATIC_CHECK(success);
    }
  }

  CHECK(emio::format("{}", foobar{42}) == "foobar: 42");
  CHECK(emio::format("{:-}", foobar{42}) == "foobar: -42");
  CHECK(emio::format(emio::runtime{"{:+}"}, foobar{42}) == emio::err::invalid_format);
}

namespace {

struct bazz0 {};
struct bazz1 {};
struct bazz2 {};
struct bazz3 {};

}  // namespace

template <>
class emio::formatter<bazz0> {
 public:
  [[nodiscard]] constexpr result<void> validate(reader<char>& rdr) noexcept;
};

template <>
class emio::formatter<bazz1> {
 public:
  static void validate();
};

template <>
class emio::formatter<bazz2> {
 public:
  void validate();
};

template <>
class emio::formatter<bazz3> {
 public:
  template <typename Char>
  [[nodiscard]] constexpr result<void> validate(reader<Char>& rdr) noexcept;
};

TEST_CASE("detail::has_validate_function_v checks", "[formatter]") {
  STATIC_CHECK(emio::detail::format::has_validate_function_v<wrap>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<wrap>);
  STATIC_CHECK(emio::detail::format::has_validate_function_v<foo>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<foo>);
  STATIC_CHECK(emio::detail::format::has_validate_function_v<bar>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<bar>);
  STATIC_CHECK(!emio::detail::format::has_validate_function_v<foobar>);
  STATIC_CHECK(!emio::detail::format::has_any_validate_function_v<foobar>);

  STATIC_CHECK(!emio::detail::format::has_validate_function_v<bazz0>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<bazz0>);

  STATIC_CHECK(!emio::detail::format::has_validate_function_v<bazz1>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<bazz1>);

  STATIC_CHECK(!emio::detail::format::has_validate_function_v<bazz2>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<bazz2>);

  STATIC_CHECK(!emio::detail::format::has_validate_function_v<bazz3>);
  STATIC_CHECK(emio::detail::format::has_any_validate_function_v<bazz3>);
}
