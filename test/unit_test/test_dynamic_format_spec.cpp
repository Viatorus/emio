// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

namespace {

class no_copy_or_move {
 public:
  no_copy_or_move() = default;
  no_copy_or_move(const no_copy_or_move&) = delete;
  no_copy_or_move(no_copy_or_move&&) = delete;
  no_copy_or_move& operator=(const no_copy_or_move&) = delete;
  no_copy_or_move& operator=(no_copy_or_move&&) = delete;
};

constexpr auto format_as(const no_copy_or_move& /*unused*/) {
  return "nothing";
}

}  // namespace

TEST_CASE("dynamic format spec") {
  SECTION("default constructed") {
    // Spec doesn't overwrite anything.
    emio::format_spec spec{};

    CHECK(emio::format("{}", spec.with(5)) == "5");
    CHECK(emio::format("{:4}", spec.with(5)) == "   5");
    CHECK(emio::format("{:f}", spec.with(5.1)) == "5.100000");
    CHECK(emio::format("{:.2f}", spec.with(5.1)) == "5.10");
  }

  SECTION("custom width and precision") {
    // Spec always overwrite.
    emio::format_spec spec{.width = 3, .precision = 3};

    CHECK(emio::format("{}", spec.with(5)) == "  5");
    CHECK(emio::format("{:4}", spec.with(5)) == "  5");
    CHECK(emio::format("{:f}", spec.with(5.1)) == "5.100");
    CHECK(emio::format("{:.0f}", spec.with(5.1)) == "5.100");
  }

  SECTION("with(...) doesn't copy or move") {
    // Object is not copied or moved because a reference is held.
    emio::format_spec spec{};

    CHECK(emio::format("{}", spec.with(no_copy_or_move{})) == "nothing");

    no_copy_or_move obj{};
    CHECK(emio::format("{}", spec.with(obj)) == "nothing");
    CHECK(emio::format("{}", spec.with(std::move(obj))) == "nothing");
  }

  SECTION("compile-time") {
    constexpr bool success = [] {
      std::array<char, 10> arr;
      emio::span_buffer buf{arr};

      emio::format_spec spec{.width = 4};
      static_cast<void>(emio::format_to(buf, "{:^}", spec.with(42)).value());
      return buf.view() == " 42 ";
    }();
    STATIC_CHECK(success);
  }
}
