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

TEST_CASE("dynamic specs") {
  SECTION("default constructed") {
    // Specs don't overwrite anything.

    emio::dynamic_spec spec{};
    CHECK(emio::format("{}", spec.with(5)) == "5");
    CHECK(emio::format("{:4}", spec.with(5)) == "   5");
    CHECK(emio::format("{:f}", spec.with(5.1)) == "5.100000");
    CHECK(emio::format("{:.2f}", spec.with(5.1)) == "5.10");
  }

  SECTION("custom width and precision") {
    // Specs always overwrite.
    emio::dynamic_spec spec{.width = 3, .precision = 3};

    CHECK(emio::format("{}", spec.with(5)) == "  5");
    CHECK(emio::format("{:4}", spec.with(5)) == "  5");
    CHECK(emio::format("{:f}", spec.with(5.1)) == "5.100");
    CHECK(emio::format("{:.0f}", spec.with(5.1)) == "5.100");
  }

  SECTION("with(...) doesn't copy or move") {
    emio::dynamic_spec spec{};

    CHECK(emio::format("{}", spec.with(no_copy_or_move{})) == "nothing");

    no_copy_or_move obj{};
    CHECK(emio::format("{}", spec.with(obj)) == "nothing");
  }
}
