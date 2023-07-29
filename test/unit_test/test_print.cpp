// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

TEST_CASE("print/println") {
  std::FILE* no_file{};

  emio::print("hello {}", "world");
  CHECK(emio::print(emio::runtime("hello {}"), "world"));
  CHECK(emio::print(stderr, "hello {}", "world"));
  CHECK(emio::print(stderr, emio::runtime("hello {}"), "world"));
  CHECK(emio::print(no_file, emio::runtime("hello {}"), "world") == emio::err::invalid_data);

  emio::println("hello {}", "world");
  CHECK(emio::println(emio::runtime("hello {}"), "world"));
  CHECK(emio::println(stderr, "hello {}", "world"));
  CHECK(emio::println(stderr, emio::runtime("hello {}"), "world"));
  CHECK(emio::println(no_file, emio::runtime("hello {}"), "world") == emio::err::invalid_data);
}

TEST_CASE("print/println to temporary file") {
  // Open a temporary file.
  std::FILE* tmpf = std::tmpfile();
  REQUIRE(tmpf);

  // Write into.
  CHECK(emio::println(tmpf, "hello {}", "world"));
  CHECK(emio::println(tmpf, "abc"));

  // Read out again.
  std::rewind(tmpf);
  std::array<char, 13> buf{};
  CHECK(std::fgets(buf.data(), buf.size(), tmpf) != nullptr);
  CHECK(std::string_view{buf.data(), 12} == "hello world\n");
  CHECK(std::fgets(buf.data(), buf.size(), tmpf) != nullptr);
  CHECK(std::string_view{buf.data(), 4} == "abc\n");
}
