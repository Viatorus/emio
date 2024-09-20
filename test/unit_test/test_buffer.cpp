// Unit under test.
#include <emio/buffer.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/generators/catch_generators_range.hpp>

namespace {

constexpr void fill(const emio::result<std::span<char>>& area, char v) {
  std::fill(area->begin(), area->end(), v);
}

template <typename T>
constexpr bool check_equality_of_buffer(T& expected, T& other, bool data_ptr_is_different) {
  bool result = true;

  std::array<char, 1> dummy{0};
  std::array<char, 1> dummy2{0};
  emio::result<std::span<char>> area{dummy};
  emio::result<std::span<char>> area2{dummy};
  if (data_ptr_is_different) {
    area2 = dummy2;
  }

  const auto compare = [&]() {
    result &= area.has_value();
    result &= area2.has_value();
    if (data_ptr_is_different) {
      result &= area->data() != area2->data();
    } else {
      result &= area->data() == area2->data();
    }
  };

  compare();

  area = expected.get_write_area_of(5);
  area2 = other.get_write_area_of(5);

  if ((!area && area2) || (area && !area2)) {
    return false;
  }
  if (!area && !area2) {
    return result;
  }

  compare();

  fill(area, 'x');
  fill(area2, 'x');

  result &= expected.view() == other.view();

  area = expected.get_write_area_of(2);
  area2 = other.get_write_area_of(2);

  if ((!area && area2) || (area && !area2)) {
    return false;
  }
  if (!area && !area2) {
    return result;
  }

  fill(area, 'y');
  fill(area2, 'z');

  if (data_ptr_is_different) {
    result &= expected.view() != other.view();
  } else {
    result &= expected.view() == other.view();
  }

  return result;
}

template <typename T>
void check_gang_of_5(T& buf, bool data_ptr_is_different) {
  SECTION("copy-construct") {
    T buf2{buf};
    CHECK(check_equality_of_buffer(buf, buf2, data_ptr_is_different));
  }
  SECTION("copy-assign") {
    T buf2;
    buf2 = buf;
    CHECK(check_equality_of_buffer(buf, buf2, data_ptr_is_different));
  }
  SECTION("move-construct") {
    T buf_tmp{buf};
    T buf2{std::move(buf_tmp)};
    CHECK(check_equality_of_buffer(buf, buf2, data_ptr_is_different));
  }
  SECTION("move-assign") {
    T buf_tmp{buf};
    T buf2;
    buf2 = std::move(buf_tmp);
    CHECK(check_equality_of_buffer(buf, buf2, data_ptr_is_different));
  }
  SECTION("wild") {
    T buf2{buf};
    T buf3{std::move(buf2)};
    T buf4;
    buf4 = buf3;
    T buf5;
    buf5 = std::move(buf4);
    T buf6{buf};
    buf6 = buf5;

    SECTION("1") {
      CHECK(check_equality_of_buffer(buf, buf5, data_ptr_is_different));
    }
    SECTION("2") {
      CHECK(check_equality_of_buffer(buf6, buf5, data_ptr_is_different));
    }
  }
  SECTION("self-assignment copy") {
    T buf2{buf};
    T& buf_tmp = buf2;  // Prevent compiler warn about self assignment.
    buf2 = buf_tmp;
    CHECK(check_equality_of_buffer(buf, buf2, data_ptr_is_different));
  }
  SECTION("self-assignment move") {
    T buf2{buf};
    T& buf_tmp = buf2;  // Prevent compiler warn about self assignment.
    buf2 = std::move(buf_tmp);
    CHECK(check_equality_of_buffer(buf, buf2, data_ptr_is_different));
  }
}

}  // namespace

TEST_CASE("memory_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a memory_buffer.
  // * Write into the buffer.
  // Expected: Everything is correctly written.

  constexpr size_t first_size{15};
  constexpr size_t second_size{55};
  constexpr size_t third_size{emio::default_cache_size};

  const std::string expected_str_part_1(first_size, 'x');
  const std::string expected_str_part_2(second_size, 'y');
  const std::string expected_str_part_3(third_size, 'z');
  const std::string expected_str = expected_str_part_1 + expected_str_part_2 + expected_str_part_3;

  const bool default_constructed = GENERATE(true, false);
  const int checkpoint = GENERATE(range(0, 5));
  INFO("Default constructed: " << default_constructed);
  INFO("Checkpoint: " << checkpoint);

  emio::memory_buffer<15> buf = default_constructed ? emio::memory_buffer<15>{} : emio::memory_buffer<15>{18};
  CHECK(buf.capacity() == (default_constructed ? 15 : 18));
  CHECK(buf.view().empty());

  if (checkpoint == 0) {
    check_gang_of_5(buf, true);
    return;
  }

  emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  CHECK(area->size() == first_size);
  fill(area, 'x');
  CHECK(buf.view().size() == first_size);
  CHECK(buf.view() == expected_str_part_1);

  if (checkpoint == 1) {
    check_gang_of_5(buf, true);
    return;
  }

  area = buf.get_write_area_of(second_size);
  REQUIRE(area);
  CHECK(area->size() == second_size);
  fill(area, 'y');
  CHECK(buf.view().size() == first_size + second_size);
  CHECK(buf.view() == expected_str_part_1 + expected_str_part_2);

  if (checkpoint == 2) {
    check_gang_of_5(buf, true);
    return;
  }

  area = buf.get_write_area_of(third_size);
  REQUIRE(area);
  CHECK(area->size() == third_size);
  fill(area, 'z');
  CHECK(buf.view() == buf.str());
  CHECK(buf.view().size() == first_size + second_size + third_size);
  CHECK(buf.view() == expected_str);

  if (checkpoint == 3) {
    check_gang_of_5(buf, true);
    return;
  }

  const size_t curr_capacity = buf.capacity();
  CHECK(curr_capacity >= buf.view().size());
  buf.reset();
  CHECK(buf.capacity() == curr_capacity);

  area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  CHECK(area->size() == first_size);

  check_gang_of_5(buf, true);
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
  CHECK(buf.capacity() == emio::default_cache_size);

  // Write a.
  for (size_t i = 0; i < default_capacity + 1U; i++) {
    buf.get_write_area_of(1).value()[0] = 'a';
  }

  // Write b.
  buf.get_write_area_of(1).value()[0] = 'b';

  // Write c.
  const emio::result<std::span<char>> area2 = buf.get_write_area_of(default_resize_capacity);
  fill(area2, 'c');

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
    result &= buf.capacity() == 1;
    result &= buf.view().empty();

    emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
    result &= area->size() == first_size;
    fill(area, 'x');
    result &= buf.view().size() == first_size;
    result &= buf.view() == "xxxxxxx";

    area = buf.get_write_area_of(second_size);
    result &= area->size() == second_size;
    fill(area, 'y');
    result &= buf.view().size() == first_size + second_size;
    result &= buf.view() == "xxxxxxxyyyyy";

    area = buf.get_write_area_of(third_size);
    result &= area->size() == third_size;
    fill(area, 'z');
    result &= buf.view().size() == first_size + second_size + third_size;
    result &= buf.view() == "xxxxxxxyyyyyzzzzzzzz";
    result &= buf.capacity() >= buf.view().size();

    const size_t curr_capacity = buf.capacity();
    result &= curr_capacity >= buf.view().size();
    buf.reset();
    result &= buf.capacity() == curr_capacity;

    area = buf.get_write_area_of(first_size);
    result &= area.has_value();
    result &= (area->size() == first_size);

    emio::memory_buffer<1> buf2{buf};
    emio::memory_buffer<1> buf3{std::move(buf2)};
    emio::memory_buffer<1> buf4;
    buf4 = buf3;
    emio::memory_buffer<1> buf5;
    buf5 = std::move(buf4);

    result &= check_equality_of_buffer(buf, buf5, true);

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
  constexpr size_t third_size{emio::default_cache_size};

  const int checkpoint = GENERATE(range(0, 6));
  INFO("Checkpoint: " << checkpoint);

  std::array<char, first_size + second_size + third_size> storage{};
  emio::span_buffer buf{storage};
  CHECK(buf.capacity() == storage.size());
  CHECK(buf.view().empty());

  if (checkpoint == 0) {
    check_gang_of_5(buf, false);
    return;
  }

  emio::result<std::span<char>> area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  CHECK(area->size() == first_size);
  CHECK(area->data() == storage.data());

  CHECK(buf.view().size() == first_size);
  CHECK(buf.view().data() == storage.data());

  if (checkpoint == 1) {
    check_gang_of_5(buf, false);
    return;
  }

  area = buf.get_write_area_of(second_size);
  REQUIRE(area);
  CHECK(area->size() == second_size);
  CHECK(area->data() == storage.data() + first_size);

  CHECK(buf.view().size() == first_size + second_size);
  CHECK(buf.view().data() == storage.data());

  if (checkpoint == 2) {
    check_gang_of_5(buf, false);
    return;
  }

  area = buf.get_write_area_of(third_size);
  REQUIRE(area);
  CHECK(area->size() == third_size);
  CHECK(area->data() == storage.data() + first_size + second_size);

  CHECK(buf.view().size() == first_size + second_size + third_size);
  CHECK(buf.view().data() == storage.data());
  CHECK(buf.view() == buf.str());

  if (checkpoint == 3) {
    check_gang_of_5(buf, false);
    return;
  }

  area = buf.get_write_area_of(1);
  REQUIRE(!area);

  area = buf.get_write_area_of(0);
  REQUIRE(area);
  CHECK(area->empty());

  if (checkpoint == 4) {
    check_gang_of_5(buf, false);
    return;
  }

  CHECK(buf.capacity() == storage.size());
  buf.reset();
  CHECK(buf.capacity() == storage.size());
  area = buf.get_write_area_of(first_size);
  REQUIRE(area);
  CHECK(area->size() == first_size);

  check_gang_of_5(buf, false);
}

TEST_CASE("span_buffer check request_write_area", "[buffer]") {
  // Test strategy:
  // * To explicit test if the span_buffer cannot provide a greater write area, we have to extend the class to access
  //   the protected function.
  // Expected: request_write_area always returns EOF.

  class dummy_span_buffer : public emio::span_buffer {
   public:
    using emio::span_buffer::request_write_area;
    using emio::span_buffer::span_buffer;
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

  const int checkpoint = GENERATE(range(0, 6));
  INFO("Checkpoint: " << checkpoint);

  emio::static_buffer<542> buf;

  if (checkpoint == 0) {
    check_gang_of_5(buf, true);
    return;
  }

  emio::result<std::span<char>> area = buf.get_write_area_of_max(500);
  REQUIRE(area);
  CHECK(area->size() == 500);
  fill(area, 'q');

  if (checkpoint == 1) {
    check_gang_of_5(buf, true);
    return;
  }

  area = buf.get_write_area_of_max(5);
  REQUIRE(area);
  CHECK(area->size() == 5);
  fill(area, 'y');

  if (checkpoint == 2) {
    check_gang_of_5(buf, true);
    return;
  }

  buf.reset();

  if (checkpoint == 3) {
    check_gang_of_5(buf, true);
    return;
  }

  area = buf.get_write_area_of_max(600);
  REQUIRE(area);
  CHECK(area->size() == 542);
  fill(area, 'l');

  if (checkpoint == 4) {
    check_gang_of_5(buf, true);
    return;
  }

  buf.reset();

  check_gang_of_5(buf, true);
}

TEST_CASE("counting_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a counting_buffer.
  // * Write into the buffer.
  // Expected: Every char written into the buffer is counted.

  constexpr size_t first_size{15};
  constexpr size_t second_size{55};
  constexpr size_t third_size{emio::default_cache_size};

  using emio::default_cache_size;

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

  area = buf.get_write_area_of(default_cache_size);
  REQUIRE(area);
  REQUIRE(area->size() == default_cache_size);

  CHECK(buf.count() == (first_size + second_size + third_size + default_cache_size));

  area = buf.get_write_area_of(default_cache_size + 1);
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

  fill(area, 'x');

  CHECK((it_buf.out() == std::next(s.data(), 2)));

  // Second write.
  area = it_buf.get_write_area_of(3);
  REQUIRE(area);
  REQUIRE(area->size() == 3);

  fill(area, 'y');

  CHECK((it_buf.out() == std::next(s.data(), 5)));

  CHECK(it_buf.flush());  // noop
  CHECK((it_buf.out() == std::next(s.data(), 5)));
  CHECK(std::string_view{s.data(), it_buf.out()} == "xxyyy");
}

TEST_CASE("iterator_buffer<iterator>", "[buffer]") {
  // Test strategy:
  // * Construct an iterator_buffer from a std::string::iterator.
  // * Write different data lengths into the buffer to test the internal buffer and flush mechanism.
  // Expected: At the end (final flush/destruction), everything is written into the string.

  const std::string expected_str_part_1(emio::default_cache_size, 'x');
  const std::string expected_str_part_2(2, 'y');
  const std::string expected_str_part_3(emio::default_cache_size, 'z');
  const std::string expected_str_part_4(3, 'x');
  const std::string expected_str_part_5(2, 'y');
  const std::string expected_str_part_6(42, 'z');
  const std::string expected_str = expected_str_part_1 + expected_str_part_2 + expected_str_part_3 +
                                   expected_str_part_4 + expected_str_part_5 + expected_str_part_6;

  std::string s(expected_str.size(), '\0');

  emio::iterator_buffer it_buf{s.begin()};

  CHECK(it_buf.out() == s.begin());
  CHECK(it_buf.get_write_area_of(std::numeric_limits<size_t>::max()) == emio::err::eof);
  CHECK(it_buf.get_write_area_of(emio::default_cache_size + 1) == emio::err::eof);

  emio::result<std::span<char>> area = it_buf.get_write_area_of(emio::default_cache_size);
  REQUIRE(area);
  REQUIRE(area->size() == emio::default_cache_size);
  fill(area, 'x');

  // No flush yet.
  CHECK(s.starts_with('\0'));

  area = it_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);
  fill(area, 'y');

  // 1. flush.
  CHECK(s.starts_with(expected_str_part_1));

  area = it_buf.get_write_area_of(emio::default_cache_size);
  REQUIRE(area);
  REQUIRE(area->size() == emio::default_cache_size);
  fill(area, 'z');

  // 2. flush.
  CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2));

  area = it_buf.get_write_area_of(3);
  REQUIRE(area);
  REQUIRE(area->size() == 3);
  fill(area, 'x');

  // 3. flush.
  CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2 + expected_str_part_3));

  area = it_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);
  fill(area, 'y');

  // No flush.
  CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2 + expected_str_part_3));

  area = it_buf.get_write_area_of(42);
  REQUIRE(area);
  REQUIRE(area->size() == 42);
  fill(area, 'z');

  // No flush.
  CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2 + expected_str_part_3));
  CHECK(s != expected_str);

  CHECK(it_buf.flush());
  CHECK(s == expected_str);
  CHECK(std::distance(it_buf.out(), s.end()) == 0);

  CHECK(it_buf.flush());
  CHECK(s == expected_str);
  CHECK(std::distance(it_buf.out(), s.end()) == 0);
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
  emio::iterator_buffer it_buf{std::back_inserter(s)};
  STATIC_CHECK(std::is_same_v<decltype(it_buf.out()), decltype(std::back_inserter(s))>);

  // First write.
  emio::result<std::span<char>> area = it_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);

  fill(area, 'x');

  CHECK(s.starts_with(expected_str_part_1));

  // Second write.
  area = it_buf.get_write_area_of(42);
  REQUIRE(area);
  REQUIRE(area->size() == 42);

  fill(area, 'y');

  CHECK(s.starts_with(expected_str_part_1 + expected_str_part_2));

  // Third write.
  area = it_buf.get_write_area_of(3);
  REQUIRE(area);
  REQUIRE(area->size() == 3);

  fill(area, 'z');

  CHECK(s.starts_with(expected_str));

  // No flush - no final content.
  CHECK(s != expected_str);

  CHECK(it_buf.flush());
  CHECK(s == expected_str);

  CHECK(it_buf.flush());
  CHECK(s == expected_str);

  CHECK_NOTHROW(it_buf.out());
}

TEST_CASE("file_buffer", "[buffer]") {
  // Test strategy:
  // * Construct a file_buffer with a temporary file stream.
  // * Write into the buffer, flush (or not) and read out again.
  // Expected: Everything is written to the file stream after flush.

  // Open a temporary file.
  std::FILE* tmpf = std::tmpfile();
  REQUIRE(tmpf);

  emio::file_buffer file_buf{tmpf};
  std::array<char, 2 * emio::default_cache_size> read_out_buf{};

  // Write area is limited.
  CHECK(file_buf.get_write_area_of(emio::default_cache_size + 1) == emio::err::eof);

  // Write into.
  emio::result<std::span<char>> area = file_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);
  fill(area, 'y');

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
  fill(area, 'z');

  // Read out (without flush).
  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 6} == std::string_view{"yy\0\0\0\0", 6});

  // Flush.
  CHECK(file_buf.flush());

  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 6} == "yyzzzz");

  const std::string expected_long_str_part(emio::default_cache_size, 'x');

  area = file_buf.get_write_area_of(emio::default_cache_size);
  REQUIRE(area);
  REQUIRE(area->size() == emio::default_cache_size);
  fill(area, 'x');

  // Internal flush should have happened.
  area = file_buf.get_write_area_of(2);
  REQUIRE(area);
  REQUIRE(area->size() == 2);
  fill(area, 'z');

  std::rewind(tmpf);
  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 6 + emio::default_cache_size} == "yyzzzz" + expected_long_str_part);
}

TEST_CASE("file_buffer reset", "[buffer]") {
  std::FILE* tmpf = std::tmpfile();
  REQUIRE(tmpf);

  emio::file_buffer file_buf{tmpf};
  std::array<char, 2 * emio::default_cache_size> read_out_buf{};

  // Write into.
  emio::result<std::span<char>> area = file_buf.get_write_area_of(4);
  REQUIRE(area);
  REQUIRE(area->size() == 4);
  fill(area, 'x');

  // Nothing should be written.
  file_buf.reset();
  CHECK(file_buf.flush());

  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf) == nullptr);

  area = file_buf.get_write_area_of(4);
  REQUIRE(area);
  REQUIRE(area->size() == 4);
  fill(area, 'y');

  // Nothing should be written.
  file_buf.reset();

  area = file_buf.get_write_area_of(4);
  REQUIRE(area);
  REQUIRE(area->size() == 4);
  fill(area, 'z');

  // Flush and then reset.
  CHECK(file_buf.flush());
  file_buf.reset();

  CHECK(std::fgets(read_out_buf.data(), read_out_buf.size(), tmpf));
  CHECK(std::string_view{read_out_buf.data(), 4} == "zzzz");
}

TEST_CASE("truncating_buffer", "[buffer]") {
  emio::static_buffer<64> primary_buf;

  SECTION("request less than limit (1)") {
    const bool explicit_flush = GENERATE(false, true);
    const std::string expected_string = std::string(40, 'a') + std::string(8, 'b');
    emio::truncating_buffer buf{primary_buf, 48};

    emio::result<std::span<char>> area = buf.get_write_area_of(40);
    REQUIRE(area);
    fill(area, 'a');
    CHECK(buf.count() == 40);

    // not flushed
    CHECK(primary_buf.view().size() == 0);
    if (explicit_flush) {
      // flushed
      CHECK(buf.flush());
      CHECK(primary_buf.view().size() == 40);
    }

    SECTION("request less than limit (2)") {
      area = buf.get_write_area_of(8);
      REQUIRE(area);
      fill(area, 'b');
      CHECK(buf.count() == 48);

      if (explicit_flush) {
        // not flushed
        CHECK(primary_buf.view().size() == 40);
        // flushed
        CHECK(buf.flush());
        CHECK(primary_buf.view().size() == 48);
      } else {
        CHECK(primary_buf.view().size() == 0);
      }

      SECTION("request more than limit (3)") {
        area = buf.get_write_area_of(emio::default_cache_size);
        REQUIRE(area);
        fill(area, 'c');
        CHECK(buf.count() == 48 + emio::default_cache_size);

        // not flushed
        CHECK(primary_buf.view().size() == 48);
        // flushed
        CHECK(buf.flush());
        CHECK(primary_buf.view().size() == 48);

        CHECK(primary_buf.view() == expected_string);
      }
    }
    SECTION("request more than limit (2)") {
      area = buf.get_write_area_of(emio::default_cache_size);
      REQUIRE(area);
      fill(area, 'b');
      CHECK(buf.count() == 40 + emio::default_cache_size);

      // not flushed
      CHECK(primary_buf.view().size() == 40);
      // flushed
      CHECK(buf.flush());
      CHECK(primary_buf.view().size() == 48);

      CHECK(primary_buf.view() == expected_string);
    }
  }
  SECTION("request more than limit (1)") {
    const std::string expected_string = std::string(48, 'a');
    emio::truncating_buffer buf{primary_buf, 48};

    emio::result<std::span<char>> area = buf.get_write_area_of(emio::default_cache_size);
    REQUIRE(area);
    fill(area, 'a');
    CHECK(buf.count() == emio::default_cache_size);

    // not flushed
    CHECK(primary_buf.view().size() == 0);
    const bool explicit_flush = GENERATE(false, true);
    if (explicit_flush) {
      // flushed
      CHECK(buf.flush());
      CHECK(primary_buf.view().size() == 48);
    }

    SECTION("request more than limit (2)") {
      CHECK(buf.get_write_area_of(emio::default_cache_size));
      CHECK(buf.count() == 2 * emio::default_cache_size);

      // not flushed
      CHECK(primary_buf.view().size() == 48);
      // flushed
      CHECK(buf.flush());
      CHECK(primary_buf.view().size() == 48);

      CHECK(primary_buf.view() == expected_string);
    }
  }
  SECTION("request more than primary") {
    emio::truncating_buffer buf{primary_buf, 86};

    SECTION("request less than limit (1)") {
      const std::string expected_string = std::string(60, 'a') + std::string(4, 'b');

      emio::result<std::span<char>> area = buf.get_write_area_of(60);
      REQUIRE(area);
      fill(area, 'a');
      CHECK(buf.count() == 60);

      // not flushed
      CHECK(primary_buf.view().size() == 0);
      const bool explicit_flush = GENERATE(false, true);
      if (explicit_flush) {
        // flushed
        CHECK(buf.flush());
        CHECK(primary_buf.view().size() == 60);
      }

      SECTION("request less than limit twice (2)") {
        area = buf.get_write_area_of(8);
        REQUIRE(area);
        fill(area, 'b');
        CHECK(buf.count() == 68);

        // Requesting more will fail because primary buffer is too small.
        CHECK_FALSE(buf.get_write_area_of(emio::default_cache_size));

        CHECK(primary_buf.view().size() == 64);
        CHECK(primary_buf.view() == expected_string);
      }
    }
    SECTION("request more than limit (1)") {
      const std::string expected_string = std::string(64, 'a');

      emio::result<std::span<char>> area = buf.get_write_area_of(90);
      REQUIRE(area);
      fill(area, 'a');
      CHECK(buf.count() == 90);

      // not flushed
      CHECK(primary_buf.view().size() == 0);
      // flush not possible because primary buffer is too small
      CHECK_FALSE(buf.flush());

      CHECK(primary_buf.view().size() == 64);
      CHECK(primary_buf.view() == expected_string);
    }
  }
  SECTION("request more than the cache size") {
    const std::string expected_string = std::string(64, 'a');
    emio::truncating_buffer buf{primary_buf, 64};

    emio::result<std::span<char>> area = buf.get_write_area_of_max(emio::default_cache_size + 10);
    REQUIRE(area);
    CHECK(area->size() == emio::default_cache_size);
    fill(area, 'a');

    // not flushed
    CHECK(primary_buf.view().size() == 0);
    // flush not possible because primary buffer is too small
    CHECK(buf.flush());

    CHECK(primary_buf.view().size() == 64);
    CHECK(primary_buf.view() == expected_string);
  }
}
