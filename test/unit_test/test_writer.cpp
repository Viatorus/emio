// Unit under test.
#include <emio/writer.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("writer", "[writer]") {
  // Test strategy:
  // * Construct a writer from a string buffer.
  // * Write data with the writer into the buffer.
  // Expected: Every method works as expected.

  emio::string_buffer buf{};
  emio::writer writer{buf};

  SECTION("get_buffer") {
    emio::buffer<char>& buffer = writer.get_buffer();
    auto area = buffer.get_write_area_of(1);
    REQUIRE(area);
    (*area)[0] = '1';
    CHECK(buf.view() == "1");
  }
  SECTION("write_char") {
    CHECK(writer.write_char('c'));
    CHECK(buf.view() == "c");
  }
  SECTION("write_char_n") {
    CHECK(writer.write_char_n('c', 3));
    CHECK(buf.view() == "ccc");
  }
  SECTION("write_char_escaped") {
    CHECK(writer.write_char_escaped('"'));
    CHECK(buf.view() == "'\\\"'");
  }
  SECTION("write_str") {
    CHECK(writer.write_str("s"));
    CHECK(buf.view() == "s");
  }
  SECTION("write_str_escaped") {
    CHECK(writer.write_str_escaped("'"));
    CHECK(buf.view() == "\"\\'\"");
  }
  SECTION("write_int") {
    CHECK(writer.write_int(1));
    CHECK(buf.view() == "1");

    CHECK(writer.write_int(15, {.base = 16}));
    CHECK(buf.view() == "1f");

    CHECK(writer.write_int(10, {.base = 11, .upper_case = true}));
    CHECK(buf.view() == "1fA");

    CHECK(writer.write_int(-3, {.base = 2}));
    CHECK(buf.view() == "1fA-11");

    CHECK(writer.write_int(3, {.base = 1}) == emio::err::invalid_argument);
    CHECK(writer.write_int(3, {.base = 37}) == emio::err::invalid_argument);
    CHECK(buf.view() == "1fA-11");
  }
}

TEST_CASE("writer with cached buffer", "[writer]") {
  // Test strategy:
  // * Construct a writer from a cached buffer (output iterator).
  // * Write long strings (> internal_buffer_size) with the writer into the buffer.
  // Expected: The string methods work as expected because the string is written in chunks.

  using emio::detail::internal_buffer_size;

  const std::string expected_str_part_1(internal_buffer_size + 1, 'x');
  const std::string expected_str_part_2(internal_buffer_size + 2, 'y');
  const std::string expected_str_part_3(internal_buffer_size + 3, 'z');

  std::string storage;
  storage.resize(5 * internal_buffer_size);
  emio::iterator_buffer buf{storage.begin()};

  emio::writer writer{buf};
  CHECK(writer.write_char_n('x', expected_str_part_1.size()));
  CHECK(writer.write_str(expected_str_part_2));
  // TODO: Make it work.
  //  CHECK(writer.write_str_escaped(expected_str_part_3));

  auto end = buf.out();
  std::string s{storage.begin(), end};

  CHECK(s == expected_str_part_1 + expected_str_part_2);
  //  CHECK(s == expected_str_part_1 + expected_str_part_2 + '"' + expected_str_part_3 + '"');
}

TEST_CASE("writer over zero buffer", "[writer]") {
  // Test strategy:
  // * Construct a writer from a zero span buffer.
  // * Write data with the writer into the buffer.
  // Expected: Every method fails with eof because of zero capacity.

  constexpr emio::err eof = emio::err::eof;

  emio::span_buffer buf{};
  emio::writer writer{buf};

  CHECK(writer.write_char('a') == eof);
  CHECK(writer.write_char_n('a', 1) == eof);
  CHECK(writer.write_char_escaped('a') == eof);
  CHECK(writer.write_str("a") == eof);
  CHECK(writer.write_str_escaped("a") == eof);
  CHECK(writer.write_int(0) == eof);
}
