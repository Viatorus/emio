// Unit under test.
#include <emio/detail/bitset.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

TEST_CASE("bitset") {
  // Test strategy:
  // * Create bitsets of different length and test all methods.
  // Expected: Methods behave as expected.

  using emio::detail::bitset;

  SECTION("0") {
    bitset<0> bitset;
    CHECK(bitset.size() == 0);
    CHECK(bitset.all());
    CHECK(bitset.all_first(0));
    CHECK(!bitset.all_first(100));
  }
  SECTION("1") {
    bitset<1> bitset;
    CHECK(bitset.size() == 1);
    CHECK(!bitset.all());
    CHECK(!bitset.all_first(1));
    CHECK(!bitset.all_first(2));

    bitset.set(0);
    CHECK(bitset.all());
    CHECK(bitset.all_first(1));
    CHECK(!bitset.all_first(2));
  }
  SECTION("8") {
    bitset<8> bitset;
    CHECK(bitset.size() == 8);
    CHECK(!bitset.all());

    for (size_t i = 0; i < 7; i++) {
      bitset.set(i);
    }
    CHECK(!bitset.all());
    CHECK(bitset.all_first(7));
    CHECK(!bitset.all_first(8));

    bitset.set(7);
    CHECK(bitset.all());
    CHECK(bitset.all_first(8));
  }
  SECTION("13") {
    bitset<13> bitset;
    CHECK(bitset.size() == 13);
    CHECK(!bitset.all());

    for (size_t i = 0; i < 12; i++) {
      bitset.set(i);
    }
    CHECK(!bitset.all());
    CHECK(bitset.all_first(12));
    CHECK(!bitset.all_first(13));

    bitset.set(12);
    CHECK(bitset.all());
    CHECK(bitset.all_first(13));
  }
  SECTION("word") {
    constexpr size_t bits_of_word = sizeof(size_t) * 8;

    bitset<bits_of_word> bitset;
    CHECK(bitset.size() == bits_of_word);
    CHECK(!bitset.all());

    for (size_t i = 0; i < bits_of_word - 1; i++) {
      bitset.set(i);
    }
    CHECK(!bitset.all());
    CHECK(bitset.all_first(bits_of_word - 1));
    CHECK(!bitset.all_first(bits_of_word));

    bitset.set(bits_of_word - 1);
    CHECK(bitset.all());
    CHECK(bitset.all_first(bits_of_word));
  }
  SECTION("123") {
    bitset<123> bitset;
    CHECK(bitset.size() == 123);
    CHECK(!bitset.all());

    for (size_t i = 0; i < 55; i++) {
      bitset.set(i);
    }
    CHECK(!bitset.all());
    CHECK(bitset.all_first(55));
    CHECK(!bitset.all_first(56));

    for (size_t i = 55; i < 122; i++) {
      bitset.set(i);
    }
    CHECK(!bitset.all());
    CHECK(bitset.all_first(122));
    CHECK(!bitset.all_first(123));

    bitset.set(122);
    CHECK(bitset.all());
    CHECK(bitset.all_first(123));
  }
  SECTION("256") {
    bitset<256> bitset;
    CHECK(bitset.size() == 256);
    CHECK(!bitset.all());

    for (size_t i = 0; i < 255; i++) {
      bitset.set(i);
    }
    CHECK(!bitset.all());
    CHECK(bitset.all_first(255));
    CHECK(!bitset.all_first(256));

    bitset.set(255);
    CHECK(bitset.all());
    CHECK(bitset.all_first(256));
  }
  SECTION("constexpr check") {
    constexpr bool success = [] {
      bitset<74> bitset;

      bool good = true;

      good &= (bitset.size() == 74);
      good &= (!bitset.all());

      for (size_t i = 0; i < 73; i++) {
        bitset.set(i);
      }
      good &= (!bitset.all());

      bitset.set(73);
      good &= (bitset.all());
      good &= (bitset.all_first(74));

      return good;
    }();
    STATIC_CHECK(success);
  }
}
