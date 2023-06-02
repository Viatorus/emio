// Unit under test.
#include <emio/result.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/matchers/catch_matchers.hpp>

TEST_CASE("result<T = int>", "[result]") {
  // Test strategy:
  // * Construct a result<int> with an integer or an error.
  // * Test all result<int> methods.
  // Expected: Every method works as expected.

  SECTION("succeeded") {
    emio::result<int> res{1};

    REQUIRE(res.has_value());
    CHECK_NOTHROW(res.assume_value() == 1);
    CHECK_NOTHROW(res.value() == 1);

    CHECK_FALSE(res.has_error());
    // CHECK_NOTHROW(res.assume_error()); // Would be illegal.
    CHECK_THROWS_AS(res.error(), emio::bad_result_access);
    CHECK_THROWS_WITH(res.error(), "no error");
  }

  SECTION("failed") {
    emio::result<int> res{emio::err::eof};

    CHECK_FALSE(res.has_value());
    // CHECK_NOTHROW(res.assume_value()); // Would be illegal.
    CHECK_THROWS_AS(res.value(), emio::bad_result_access);
    CHECK_THROWS_WITH(res.value(), "eof");

    REQUIRE(res.has_error());
    CHECK(res.assume_error() == emio::err::eof);
    CHECK(res.error() == emio::err::eof);
  }

  SECTION("comparison") {
    emio::result<int> res_with_1{1};
    emio::result<int> res_with_2{2};
    emio::result<int> res_with_error_1{emio::err::eof};
    emio::result<int> res_with_error_2{emio::err::invalid_argument};

    CHECK(res_with_1 == res_with_1);
    CHECK_FALSE(res_with_1 != res_with_1);

    CHECK(res_with_error_1 == res_with_error_1);
    CHECK_FALSE(res_with_error_1 != res_with_error_1);

    CHECK_FALSE(res_with_error_1 == res_with_error_2);
    CHECK(res_with_error_1 != res_with_error_2);

    CHECK_FALSE(res_with_1 == res_with_error_1);
    CHECK_FALSE(res_with_error_1 == res_with_1);
    CHECK(res_with_1 != res_with_error_1);
    CHECK(res_with_error_1 != res_with_1);

    CHECK_FALSE(res_with_1 == res_with_2);
    CHECK(res_with_1 != res_with_2);

    CHECK(res_with_1 == 1);
    CHECK(1 == res_with_1);
    CHECK_FALSE(res_with_1 != 1);
    CHECK_FALSE(1 != res_with_1);

    CHECK_FALSE(res_with_2 == 1);
    CHECK_FALSE(1 == res_with_2);
    CHECK(res_with_2 != 1);
    CHECK(1 != res_with_2);

    CHECK_FALSE(res_with_1 == emio::err::eof);
    CHECK(res_with_1 != emio::err::eof);
    CHECK_FALSE(res_with_error_1 == 1);
    CHECK(res_with_error_1 != 1);
  }
}

TEST_CASE("result<std::string>", "[result]") {
  // Test strategy:
  // * Construct a result<std::string> with success.
  // * Test all constructor and value access methods with different value categories.
  // Expected: Every method works as expected.

  constexpr std::string_view initial{"abc"};

  emio::result<std::string> res{initial};
  REQUIRE(res.has_value());
  CHECK(res == initial);

  SECTION("mutable lvalue") {
    res.value().clear();
    res.assume_value() += 'd';
    res->push_back('e');
    *res += 'f';

    CHECK(res == "def");
  }

  SECTION("const lvalue") {
    const emio::result<std::string> const_res = res;

    CHECK(const_res.value().size() == 3);
    CHECK(const_res.assume_value().size() == 3);
    CHECK(const_res->size() == 3);
    CHECK((*const_res).size() == 3);
  }

  SECTION("mutable rvalue") {
    std::string value;

    SECTION("assume_value") {
      value = std::move(res).assume_value();
    }
    SECTION("value") {
      value = std::move(res).value();
    }
    SECTION("operator*") {
      value = *std::move(res);
    }

    CHECK(value != res);
    CHECK(value == initial);
    CHECK(res->empty());
  }

  SECTION("const rvalue") {
    const emio::result<std::string> const_res = res;

    CHECK(std::move(const_res).value().size() == 3);
    CHECK(std::move(const_res).assume_value().size() == 3);
    CHECK(std::move(const_res)->size() == 3);
    CHECK((*std::move(const_res)).size() == 3);
  }

  SECTION("value_or") {
    SECTION("when *this has a value") {
      emio::result<std::string> s{"abc"};

      CHECK(s.value_or("def") == "abc");
      CHECK(std::move(s).value_or("def") == "abc");
    }
    SECTION("when *this has no value") {
      emio::result<std::string> s{emio::err::invalid_data};

      CHECK(s.value_or("def") == "def");
      CHECK(std::move(s).value_or("def") == "def");
    }
  }
}

TEST_CASE("emio::err to_string()", "[result]") {
  // Test strategy:
  // * Call to_string(emio::err) with all defined error codes.
  // Expected: Correct string is returned.

  CHECK(to_string(emio::err::eof) == "eof");
  CHECK(to_string(emio::err::invalid_argument) == "invalid argument");
  CHECK(to_string(emio::err::invalid_data) == "invalid data");
  CHECK(to_string(emio::err::invalid_format) == "invalid format");
  CHECK(to_string(emio::err::out_of_range) == "out of range");
}

TEST_CASE("EMIO_TRY/V macro", "[result]") {
  // Test strategy:
  // * Call different lambda function returning either a success or an error.
  // * Use EMIO_TRY or EMIO_TRYV for error handling.
  // Expected: Success, value and error is correctly propagate.

  const bool should_succeed = GENERATE(true, false);

  auto func_return_float = [=]() -> emio::result<float> {
    if (should_succeed) {
      return 1.1F;
    }
    return emio::err::eof;
  };

  if (should_succeed) {
    CHECK(func_return_float() == 1.1F);
  } else {
    CHECK(func_return_float() == emio::err::eof);
  }

  auto func_return_int = [=]() -> emio::result<int> {
    EMIO_TRYV(func_return_float());
    return 1;
  };

  if (should_succeed) {
    CHECK(func_return_int() == 1);
  } else {
    CHECK(func_return_int() == emio::err::eof);
  }

  auto func_return_char = [=]() -> emio::result<char> {
    EMIO_TRYV(func_return_int());
    return '1';
  };

  if (should_succeed) {
    CHECK(func_return_char() == '1');
  } else {
    CHECK(func_return_char() == emio::err::eof);
  }

  auto func_return_void = [=]() -> emio::result<void> {
    EMIO_TRY(char g, func_return_char());
    if (g == '1') {
      return emio::success;
    } else {
      return emio::err::invalid_data;
    }
  };

  if (should_succeed) {
    CHECK(func_return_void());
  } else {
    CHECK(func_return_void() == emio::err::eof);
  }
}
