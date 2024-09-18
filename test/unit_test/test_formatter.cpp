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
  constexpr result<void> format(writer& out, const wrap& arg) const noexcept {
    return formatter<int>::format(out, arg.id);
  }
};

TEST_CASE("formatter with inherit from emio::formatter", "[formatter]") {
  // Test strategy:
  // * Format a custom type with a formatter which inherit from a base emio::formatter.
  // Expected: The formatting works and the format options from the base emio::formatter can be used.

  SECTION("compile-time") {
    SECTION("simple") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

        static_cast<void>(emio::format_to(buf, "{}", wrap{42}).value());
        return buf.view() == "42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("complex") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

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
  static constexpr result<void> validate(reader& rdr) noexcept {
    EMIO_TRY(const char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      return rdr.read_if_match_char('}');
    }
    return emio::err::invalid_format;
  }

  constexpr result<void> parse(reader& rdr) noexcept {
    const char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      sign_ = -1;
    }
    rdr.pop();
    return success;
  }

  constexpr result<void> format(writer& out, const foo& arg) const noexcept {
    return emio::format_to(out.get_buffer(), "foo: {}", sign_ * arg.id);
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
        emio::static_buffer<10> buf{};

        static_cast<void>(emio::format_to(buf, "{}", foo{42}).value());
        return buf.view() == "foo: 42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("advanced") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

        static_cast<void>(emio::format_to(buf, "{:-}", foo{42}).value());
        return buf.view() == "foo: -42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("error") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

        return emio::format_to(buf, emio::runtime("{:+}"), foo{42}) == emio::err::invalid_format;
      }();
      STATIC_CHECK(success);
    }
  }

  CHECK(emio::format("{}", foo{42}) == "foo: 42");
  CHECK(emio::format("{:-}", foo{42}) == "foo: -42");
  CHECK(emio::format(emio::runtime("{:+}"), foo{42}) == emio::err::invalid_format);
}

namespace {

struct bar {};

}  // namespace

template <>
class emio::formatter<bar> {
 public:
  static result<void> validate(reader& rdr) noexcept {
    EMIO_TRY(const char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      return rdr.read_if_match_char('}');
    }
    return emio::err::invalid_format;
  }

  result<void> parse(reader& rdr) noexcept {
    const char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      upper_case_ = true;
    }
    rdr.pop();
    return success;
  }

  result<void> format(writer& out, const bar& /*arg*/) const noexcept {
    if (upper_case_) {
      return out.write_str("BAR");
    }
    return out.write_str("bar");
  }

 private:
  bool upper_case_{false};
};

TEST_CASE("formatter with non-constexpr methods", "[formatter]") {
  // Test strategy:
  // * Format a custom type with an own formatter providing all necessary non-constexpr methods.
  // Expected: The formatting works.

  CHECK(emio::format(emio::runtime("{}"), bar{}) == "bar");
  CHECK(emio::format(emio::runtime("{:-}"), bar{}) == "BAR");
  CHECK(emio::format(emio::runtime("{:+}"), bar{}) == emio::err::invalid_format);
}

namespace {

struct foobar {
  int id;
};

}  // namespace

template <>
class emio::formatter<foobar> {
 public:
  constexpr result<void> parse(reader& rdr) noexcept {
    EMIO_TRY(const char c, rdr.read_char());
    if (c == '}') {  // Format end.
      return success;
    }
    if (c == '-') {
      sign_ = -1;
      return rdr.read_if_match_char('}');
    }
    return emio::err::invalid_format;
  }

  constexpr result<void> format(writer& out, const foobar& arg) const noexcept {
    return emio::format_to(out.get_buffer(), "foobar: {}", sign_ * arg.id);
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
        emio::static_buffer<10> buf{};

        static_cast<void>(emio::format_to(buf, "{}", foobar{42}).value());
        return buf.view() == "foobar: 42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("advanced") {
      constexpr bool success = [] {
        emio::static_buffer<11> buf{};

        static_cast<void>(emio::format_to(buf, "{:-}", foobar{42}).value());
        return buf.view() == "foobar: -42";
      }();
      STATIC_CHECK(success);
    }
    SECTION("error") {
      constexpr bool success = [] {
        emio::static_buffer<10> buf{};

        return emio::format_to(buf, emio::runtime("{:+}"), foobar{42}) == emio::err::invalid_format;
      }();
      STATIC_CHECK(success);
    }
  }

  CHECK(emio::format("{}", foobar{42}) == "foobar: 42");
  CHECK(emio::format("{:-}", foobar{42}) == "foobar: -42");
  CHECK(emio::format(emio::runtime("{:+}"), foobar{42}) == emio::err::invalid_format);
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
  constexpr result<void> validate(reader& rdr) noexcept;
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
  constexpr result<void> validate(reader& rdr) noexcept;
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

namespace {
struct f1 {};
struct f2 {};
struct f3 {};
}  // namespace

template <>
class emio::formatter<f1> {
 public:
};

template <>
class emio::formatter<f2> {
 public:
  static constexpr bool format_can_fail = false;
};

template <>
class emio::formatter<f3> {
 public:
  static constexpr bool format_can_fail = true;
};

TEST_CASE("emio::format_can_fail checks", "[formatter]") {
  STATIC_CHECK_FALSE(emio::format_can_fail_v<f1>);
  STATIC_CHECK_FALSE(emio::format_can_fail_v<f2>);
  STATIC_CHECK(emio::format_can_fail_v<f3>);
}
