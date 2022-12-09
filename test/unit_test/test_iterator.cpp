// Unit under test.
#include <emio/iterator.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("truncating_iterator") {
  // Test strategy:
  // * Construct a truncating iterator from different types of std::string iterators.
  // * Copy a bigger string into the limited truncating iterator.
  // Expected: Only a part of the bigger string has been copied into the std::string.

  std::string str;
  constexpr size_t max_length = 10;
  constexpr std::string_view full_message = "acbx40!lqcl.qxon0241";
  STATIC_REQUIRE(full_message.size() == 20);

  SECTION("raw ptr") {
    str.resize(max_length);

    auto* it = str.data();
    emio::truncating_iterator ti{it, max_length};

    for (char c : full_message) {
      *ti++ = c;
    }

    CHECK(ti.count() == 10);
    CHECK(std::distance(ti.out(), &*str.end()) == 0);
    CHECK(str.size() == 10);
    CHECK(str == full_message.substr(0, max_length));

    // Possible API calls.
    char* out = ti.out();
    static_cast<void>(out);
    ti++;
    ++ti;
    *ti = '#';
  }

  SECTION("iterator") {
    str.resize(10);

    auto it = str.begin();
    emio::truncating_iterator ti{it, max_length};

    for (char c : full_message) {
      *ti++ = c;
    }

    CHECK(ti.count() == 10);
    CHECK(std::distance(ti.out(), str.end()) == 0);
    CHECK(str.size() == 10);
    CHECK(str == full_message.substr(0, max_length));

    // Possible API calls.
    std::string::iterator out = ti.out();
    static_cast<void>(out);
    ti++;
    ++ti;
    *ti = '#';
  }

  SECTION("back_insert_iterator") {
    auto it = std::back_inserter(str);
    emio::truncating_iterator ti{it, max_length};

    CHECK(str.empty());

    for (char c : full_message) {
      *ti++ = c;
    }

    CHECK(ti.count() == 10);
    CHECK(str.size() == 10);
    CHECK(str == full_message.substr(0, max_length));

    // Possible API calls.
    std::back_insert_iterator<std::string> out = ti.out();
    static_cast<void>(out);
    ti++;
    ++ti;
    *ti = '#';
    ti = '#';
  }
}
