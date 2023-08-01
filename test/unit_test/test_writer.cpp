// Unit under test.
#include <emio/writer.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

namespace {

class buffer_from_cb : public emio::buffer {
 public:
  void set_cache_cb(std::function<std::span<char>()> cache_cb) noexcept {
    cache_cb_ = std::move(cache_cb);
  }

  emio::result<std::span<char>> request_write_area(const size_t /*used*/, const size_t size) noexcept override {
    const std::span<char> area{cache_cb_()};
    if (area.empty()) {
      return emio::err::eof;
    }
    this->set_write_area(area);
    if (size > area.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  std::function<std::span<char>()> cache_cb_;
};

}  // namespace

TEST_CASE("writer", "[writer]") {
  // Test strategy:
  // * Construct a writer from a string buffer.
  // * Write data with the writer into the buffer.
  // Expected: Every method works as expected.

  emio::memory_buffer buf{};
  emio::writer writer{buf};

  SECTION("get_buffer") {
    emio::buffer& buffer = writer.get_buffer();
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
    SECTION("normal") {
      CHECK(writer.write_str_escaped("'"));
      CHECK(buf.view() == "\"\\'\"");
    }
    SECTION("empty") {
      CHECK(writer.write_str_escaped(""));
      CHECK(buf.view() == "\"\"");
    }
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
  CHECK(writer.write_str_escaped(expected_str_part_3));

  auto end = buf.out();
  std::string s{storage.begin(), end};

  CHECK(s == expected_str_part_1 + expected_str_part_2 + '"' + expected_str_part_3 + '"');
}

TEST_CASE("writer with cached buffer - write_char_escaped", "[writer]") {
  // Test strategy:
  // * Construct a writer from a cached buffer.
  // * Write an escaped char but only provide different sizes of chunks from the buffer.
  // Expected: The escape method (same as write_str_escaped()) works as expected.

  constexpr char char_to_escape = '\n';
  constexpr std::string_view expected_str = "'\\n'";

  buffer_from_cb buf;
  buf.set_cache_cb([]() -> std::span<char> {
    return {};
  });

  emio::writer writer{buf};
  CHECK(writer.write_char('c') == emio::err::eof);

  size_t buffer_calls_cnt = 0;
  std::vector<char> chunks;

  SECTION("provide one char per chunk") {
    buf.set_cache_cb([&]() -> std::span<char> {
      buffer_calls_cnt += 1;
      chunks.push_back('\0');
      return {&chunks.back(), 1};
    });
    CHECK(writer.write_char_escaped(char_to_escape));
    CHECK(buffer_calls_cnt == 4);
    REQUIRE(chunks.size() == 4);
  }
  SECTION("provide two chars per chunk") {
    buf.set_cache_cb([&]() -> std::span<char> {
      buffer_calls_cnt += 1;
      chunks.push_back('\0');
      chunks.push_back('\0');
      return {&chunks.back() - 1, 2};
    });
    CHECK(writer.write_char_escaped(char_to_escape));
    CHECK(buffer_calls_cnt == 2);
    REQUIRE(chunks.size() == 4);
  }
  SECTION("provide three chars per chunk") {
    buf.set_cache_cb([&]() -> std::span<char> {
      buffer_calls_cnt += 1;
      chunks.push_back('\0');
      chunks.push_back('\0');
      chunks.push_back('\0');
      return {&chunks.back() - 2, 3};
    });
    CHECK(writer.write_char_escaped(char_to_escape));
    CHECK(buffer_calls_cnt == 2);
    REQUIRE(chunks.size() == 6);
  }
  SECTION("provide four chars per chunk") {
    buf.set_cache_cb([&]() -> std::span<char> {
      buffer_calls_cnt += 1;
      chunks.push_back('\0');
      chunks.push_back('\0');
      chunks.push_back('\0');
      chunks.push_back('\0');
      return {&chunks.back() - 3, 4};
    });
    CHECK(writer.write_char_escaped('\n'));
    CHECK(buffer_calls_cnt == 1);
    REQUIRE(chunks.size() == 4);
  }
  // Check written result.
  REQUIRE(chunks.size() >= 4);
  CHECK(std::string_view{chunks.data(), 4} == expected_str);
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
