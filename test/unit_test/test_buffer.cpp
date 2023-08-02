// Unit under test.
#include <emio/buffer.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

// TODO: Unify test data. Some tests just pop these sizes. Other tests write into the buffer.

TEST_CASE("memory_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a memory_buffer.
  // * Write into the buffer.
  // Expected: Everything is correctly written.

  constexpr size_t first_size{15};
  constexpr size_t second_size{55};
  constexpr size_t third_size{256};

  const std::string expected_str_part_1(first_size, 'x');
  const std::string expected_str_part_2(second_size, 'y');
  const std::string expected_str_part_3(third_size, 'z');
  const std::string expected_str = expected_str_part_1 + expected_str_part_2 + expected_str_part_3;

  const bool default_constructed = GENERATE(true, false);
  INFO("Default constructed: " << default_constructed);

  emio::memory_buffer<15> buf = default_constructed ? emio::memory_buffer<15>{} : emio::memory_buffer<15>{18};
  CHECK(buf.view().empty());

  emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  CHECK(area->size() == first_size);
  std::fill(area->begin(), area->end(), 'x');
  CHECK(buf.view().size() == first_size);
  CHECK(buf.view() == expected_str_part_1);

  area = buf.get_write_area_of(second_size);
  REQUIRE(area);
  CHECK(area->size() == second_size);
  std::fill(area->begin(), area->end(), 'y');
  CHECK(buf.view().size() == first_size + second_size);
  CHECK(buf.view() == expected_str_part_1 + expected_str_part_2);

  area = buf.get_write_area_of(third_size);
  REQUIRE(area);
  CHECK(area->size() == third_size);
  std::fill(area->begin(), area->end(), 'z');
  CHECK(buf.view() == buf.str());
  CHECK(buf.view().size() == first_size + second_size + third_size);
  CHECK(buf.view() == expected_str);
}

TEST_CASE("memory_buffer regression bug 1", "[buffer]") {
  // Regression description:
  // memory_buffer::request_write_area(...) passed an invalid too large write area to buffer::set_write_area(...).

  // Test strategy:
  // * Construct a memory_buffer and write 'a's into it for more than the default capacity of an std::string to
  //   trigger a resize.
  // * Write a 'b'. This memory location was a buffer overflow and not seen after another resize.
  // * Write enough 'c's into it to trigger another resize. Otherwise, the issue would only be visible with memory
  //   checks.
  // Expected: Everything is correctly written.

  const size_t default_capacity = std::string{}.capacity();
  const size_t default_resize_capacity = [] {
    std::string s;
    s.resize(s.capacity() + 1U);
    return s.capacity();
  }();

  emio::memory_buffer buf;

  // Write a.
  for (size_t i = 0; i < default_capacity + 1U; i++) {
    buf.get_write_area_of(1).value()[0] = 'a';
  }

  // Write b.
  buf.get_write_area_of(1).value()[0] = 'b';

  // Write c.
  auto area2 = buf.get_write_area_of(default_resize_capacity).value();
  std::fill(area2.begin(), area2.end(), 'c');

  std::string expected_str(default_capacity + 1, 'a');
  expected_str += 'b';
  std::fill_n(std::back_inserter(expected_str), default_resize_capacity, 'c');

  CHECK(buf.str() == expected_str);
}

TEST_CASE("memory_buffer at compile-time", "[buffer]") {
  // Test strategy:
  // * Construct a ct_memory_buffer.
  // * Write into the buffer at compile time.
  // Expected: Everything is correctly written.

  constexpr size_t first_size{7};
  constexpr size_t second_size{5};
  constexpr size_t third_size{8};

  constexpr bool success = [] {
    bool result = true;

    emio::memory_buffer<1> buf{};
    result &= buf.view().empty();

    emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
    result &= area->size() == first_size;
    std::fill(area->begin(), area->end(), 'x');
    result &= buf.view().size() == first_size;
    result &= buf.view() == "xxxxxxx";

    area = buf.get_write_area_of(second_size);
    result &= area->size() == second_size;
    std::fill(area->begin(), area->end(), 'y');
    result &= buf.view().size() == first_size + second_size;
    result &= buf.view() == "xxxxxxxyyyyy";

    area = buf.get_write_area_of(third_size);
    result &= area->size() == third_size;
    std::fill(area->begin(), area->end(), 'z');
    result &= buf.view().size() == first_size + second_size + third_size;
    result &= buf.view() == "xxxxxxxyyyyyzzzzzzzz";

    return result;
  }();
  STATIC_CHECK(success);
}

TEST_CASE("span_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a span_buffer from an std::array.
  // * Write into the buffer.
  // Expected: Everything is directly written to the array.

  constexpr size_t first_size{15};
  constexpr size_t second_size{55};
  constexpr size_t third_size{256};

  std::array<char, first_size + second_size + third_size> storage{};
  emio::span_buffer buf{storage};
  CHECK(buf.view().empty());

  emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  CHECK(area->size() == first_size);
  CHECK(area->data() == storage.data());

  CHECK(buf.view().size() == first_size);
  CHECK(buf.view().data() == storage.data());

  area = buf.get_write_area_of(second_size);
  REQUIRE(area);
  CHECK(area->size() == second_size);
  CHECK(area->data() == storage.data() + first_size);

  CHECK(buf.view().size() == first_size + second_size);
  CHECK(buf.view().data() == storage.data());

  area = buf.get_write_area_of(third_size);
  REQUIRE(area);
  CHECK(area->size() == third_size);
  CHECK(area->data() == storage.data() + first_size + second_size);

  CHECK(buf.view().size() == first_size + second_size + third_size);
  CHECK(buf.view().data() == storage.data());
  CHECK(buf.view() == buf.str());

  area = buf.get_write_area_of(1);
  REQUIRE(!area);

  area = buf.get_write_area_of(0);
  REQUIRE(area);
  CHECK(area->empty());
}

TEST_CASE("span_buffer check request_write_area", "[buffer]") {
  // Test strategy:
  // * To explicit test if the span_buffer cannot provide a greater write area, we have to extend the class to access
  //   the protected function.
  // Expected: request_write_area always returns EOF.

  class dummy_span_buffer : public emio::span_buffer {
   public:
    using emio::span_buffer::span_buffer;

    using emio::span_buffer::request_write_area;
  };

  SECTION("default constructed") {
    dummy_span_buffer buf;
    CHECK(buf.get_write_area_of_max(100) == emio::err::eof);
    CHECK(buf.request_write_area(0, 0) == emio::err::eof);
  }

  SECTION("constructed with storage") {
    std::array<char, 50> storage;
    dummy_span_buffer buf{storage};
    CHECK(buf.get_write_area_of_max(100).value().size() == 50);
    CHECK(buf.request_write_area(0, 0) == emio::err::eof);
  }
}

TEST_CASE("buffer regression bug 1", "[buffer]") {
  // Regression description:
  // buffer::get_write_area_of_max(...) tried to request more data from the concrete buffer implementation if the
  // requested size is greater than available. For fixed size buffers (e.g. span buffer) this will always return EOF.
  // Instead, the remaining bytes should be returned.

  // Test strategy:
  // * Construct a span_buffer over an std::array and request more bytes than available through get_write_area_of_max.
  // Expected: A write area matching the std::array size is returned.

  std::array<char, 50> arr;
  emio::span_buffer buf{arr};
  emio::result<std::span<char>> area = buf.get_write_area_of_max(60);
  REQUIRE(area);
  CHECK(area->size() == 50);

  // No write area available anymore.
  area = buf.get_write_area_of_max(60);
  REQUIRE(!area);

  // Except for zero bytes.
  area = buf.get_write_area_of_max(0);
  REQUIRE(area);
  CHECK(area->empty());
}

TEST_CASE("static_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a static buffer.
  // * Check max size of the buffer.
  // Expected: The max size is correct.

  emio::static_buffer<542> buffer;
  CHECK(buffer.get_write_area_of_max(600)->size() == 542);
}

TEST_CASE("counting_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a counting_buffer.
  // * Write into the buffer.
  // Expected: Every char written into the buffer is counted.

  constexpr size_t first_size{15};
  constexpr size_t second_size{55};
  constexpr size_t third_size{256};

  using emio::detail::internal_buffer_size;

  emio::detail::counting_buffer buf;
  CHECK(buf.count() == 0);

  emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  REQUIRE(area->size() == first_size);

  CHECK(buf.count() == first_size);

  area = buf.get_write_area_of(second_size);
  REQUIRE(area);
  REQUIRE(area->size() == second_size);

  CHECK(buf.count() == first_size + second_size);

  area = buf.get_write_area_of(third_size);
  REQUIRE(area);
  REQUIRE(area->size() == third_size);

  CHECK(buf.count() == (first_size + second_size + third_size));

  area = buf.get_write_area_of(internal_buffer_size);
  REQUIRE(area);
  REQUIRE(area->size() == internal_buffer_size);

  CHECK(buf.count() == (first_size + second_size + third_size + internal_buffer_size));

  area = buf.get_write_area_of(internal_buffer_size + 1);
  CHECK(!area);
  CHECK(buf.count() == (first_size + second_size + 2 * third_size));
}

TEST_CASE("iterator_buffer<char ptr>", "[buffer]") {
  // Test strategy:
  // * Construct an iterator_buffer from a char pointer.
  // * Write into the buffer.
  // Expected: Everything is directly written to the char pointer.

  std::array<char, 5> s{};
  emio::iterator_buffer it_buf{s.data()};

  // First write.
  emio::result<std::span<char>> area = it_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);

  std::fill(area->begin(), area->end(), 'x');

  CHECK((it_buf.out() == std::next(s.data(), 2)));

  // Second write.
  area = it_buf.get_write_area_of(3);
  REQUIRE(area);
  REQUIRE(area->size() == 3);

  std::fill(area->begin(), area->end(), 'y');

  CHECK((it_buf.out() == std::next(s.data(), 5)));

  it_buf.flush();  // noop
  CHECK((it_buf.out() == std::next(s.data(), 5)));
  CHECK(std::string_view{s.data(), it_buf.out()} == "xxyyy");
}

TEST_CASE("iterator_buffer<iterator>", "[buffer]") {
  // Test strategy:
  // * Construct an iterator_buffer from a std::string::iterator.
  // * Write different data lengths into the buffer to test the internal buffer and flush mechanism.
  // Expected: At the end (final flush/destruction), everything is written into the string.

  using emio::detail::internal_buffer_size;

  const std::string expected_str_part_1(internal_buffer_size, 'x');
  const std::string expected_str_part_2(2, 'y');
  const std::string expected_str_part_3(internal_buffer_size, 'z');
  const std::string expected_str_part_4(3, 'x');
  const std::string expected_str_part_5(2, 'y');
  const std::string expected_str_part_6(42, 'z');
  const std::string expected_str = expected_str_part_1 + expected_str_part_2 + expected_str_part_3 +
                                   expected_str_part_4 + expected_str_part_5 + expected_str_part_6;

  std::string s(expected_str.size(), '\0');

  {
    emio::iterator_buffer it_buf{s.begin()};

    CHECK(it_buf.out() == s.begin());
    CHECK(it_buf.get_write_area_of(std::numeric_limits<size_t>::max()) == emio::err::eof);
    CHECK(it_buf.get_write_area_of(internal_buffer_size + 1) == emio::err::eof);

    emio::result<std::span<char>> area = it_buf.get_write_area_of(internal_buffer_size);
    REQUIRE(area);
    REQUIRE(area->size() == internal_buffer_size);
    std::fill(area->begin(), area->end(), 'x');

    // No flush yet.
    CHECK(s.starts_with('\0'));

    area = it_buf.get_write_area_of(2);
    REQUIRE(area);
    REQUIRE(area->size() == 2);
    std::fill(area->begin(), area->end(), 'y');

    // 1. flush.
    CHECK(s.starts_with(expected_str_part_1));

    area = it_buf.get_write_area_of(internal_buffer_size);
    REQUIRE(area);
    REQUIRE(area->size() == internal_buffer_size);
    std::fill(area->begin(), area->end(), 'z');

    // 2. flush.
    CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2));

    area = it_buf.get_write_area_of(3);
    REQUIRE(area);
    REQUIRE(area->size() == 3);
    std::fill(area->begin(), area->end(), 'x');

    // 3. flush.
    CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2 + expected_str_part_3));

    area = it_buf.get_write_area_of(2);
    REQUIRE(area);
    REQUIRE(area->size() == 2);
    std::fill_n(area->begin(), 2, 'y');

    // No flush.
    CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2 + expected_str_part_3));

    area = it_buf.get_write_area_of(42);
    REQUIRE(area);
    REQUIRE(area->size() == 42);
    std::fill_n(area->begin(), 42, 'z');

    // No flush.
    CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2 + expected_str_part_3));
    CHECK(s != expected_str);

    const bool early_return = GENERATE(true, false);
    if (!early_return) {  // Test flush is called at buffer destruction.
      it_buf.flush();
      CHECK(s == expected_str);
      CHECK(std::distance(it_buf.out(), s.end()) == 0);

      it_buf.flush();
      CHECK(s == expected_str);
      CHECK(std::distance(it_buf.out(), s.end()) == 0);
    }
  }

  CHECK(s == expected_str);
}

TEST_CASE("iterator_buffer<back_insert_iterator>", "[buffer]") {
  // Test strategy:
  // * Construct an iterator_buffer from a char pointer.
  // * Write into the buffer.
  // Expected: Everything is directly written to the string.

  const std::string expected_str_part_1(2, 'x');
  const std::string expected_str_part_2(42, 'y');
  const std::string expected_str_part_3(3, 'z');
  const std::string expected_str = expected_str_part_1 + expected_str_part_2 + expected_str_part_3;

  std::string s;

  {
    emio::iterator_buffer it_buf{std::back_inserter(s)};
    STATIC_CHECK(std::is_same_v<decltype(it_buf.out()), decltype(std::back_inserter(s))>);

    // First write.
    emio::result<std::span<char>> area = it_buf.get_write_area_of(2);
    REQUIRE(area);
    REQUIRE(area->size() == 2);

    std::fill(area->begin(), area->end(), 'x');

    CHECK(s.starts_with(expected_str_part_1));

    // Second write.
    area = it_buf.get_write_area_of(42);
    REQUIRE(area);
    REQUIRE(area->size() == 42);

    std::fill(area->begin(), area->end(), 'y');

    CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2));

    // Third write.
    area = it_buf.get_write_area_of(3);
    REQUIRE(area);
    REQUIRE(area->size() == 3);

    std::fill(area->begin(), area->end(), 'z');

    CHECK(s.starts_with(expected_str));

    // No flush - no final content.
    CHECK(s != expected_str);

    const bool early_return = GENERATE(true, false);
    if (!early_return) {  // Test flush is called at buffer destruction.
      it_buf.flush();
      CHECK(s == expected_str);

      it_buf.flush();
      CHECK(s == expected_str);

      CHECK_NOTHROW(it_buf.out());
    }
  }

  CHECK(s == expected_str);
}

TEST_CASE("file_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a file_buffer with a temporary file stream.
  // * Write into the buffer, flush (or not) and read out again.
  // Expected: Everything is written to the file stream after flush.

  using emio::detail::internal_buffer_size;

  // Open a temporary file.
  std::FILE* tmpf = std::tmpfile();
  REQUIRE(tmpf);

  emio::file_buffer file_buf{tmpf};
  std::array<char, 2 * internal_buffer_size> read_out_buf{};

  // Write area is limited.
  CHECK(file_buf.get_write_area_of(internal_buffer_size + 1) == emio::err::eof);

  // Write into.
  emio::result<std::span<char>> area = file_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);
  std::fill(area->begin(), area->end(), 'y');

  // Read out (without flush).
  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf) == nullptr);
  CHECK(std::string_view{read_out_buf.data(), 4} == std::string_view{"\0\0\0\0", 4});

  // Flush.
  CHECK(file_buf.flush());

  // Read out (after flush).
  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 4} == std::string_view{"yy\0\0", 4});

  // Write into again.
  area = file_buf.get_write_area_of(4);
  REQUIRE(area);
  REQUIRE(area->size() == 4);
  std::fill(area->begin(), area->end(), 'z');

  // Read out (without flush).
  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 6} == std::string_view{"yy\0\0\0\0", 6});

  // Flush.
  CHECK(file_buf.flush());

  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 6} == "yyzzzz");

  const std::string expected_long_str_part(internal_buffer_size, 'x');

  area = file_buf.get_write_area_of(internal_buffer_size);
  REQUIRE(area);
  REQUIRE(area->size() == internal_buffer_size);
  std::fill(area->begin(), area->end(), 'x');

  // Internal flush should have happened.
  area = file_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);
  std::fill(area->begin(), area->end(), 'z');

  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 6 + internal_buffer_size} == "yyzzzz" + expected_long_str_part);
}
