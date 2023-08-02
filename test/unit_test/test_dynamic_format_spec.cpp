// Unit under test.
#include <emio/format.hpp>
#include <emio/ranges.hpp>

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
      emio::static_buffer<10> buf{};

      emio::format_spec spec{.width = 4};
      static_cast<void>(emio::format_to(buf, "{:^}", spec.with(42)).value());
      return buf.view() == " 42 ";
    }();
    STATIC_CHECK(success);
  }

  SECTION("works with contiguous ranges") {
    emio::format_spec spec{.precision = 2};

    std::array<double, 3> values{2.099, 3.245, 4.5};
    CHECK(emio::format("{::f}", spec.with(values)) == "[2.10, 3.25, 4.50]");
  }

  SECTION("with runtime format string") {
    emio::format_spec spec{.precision = 1};

    CHECK(emio::format(emio::runtime("{:f}"), spec.with(3.14)).value() == "3.1");
  }
}
