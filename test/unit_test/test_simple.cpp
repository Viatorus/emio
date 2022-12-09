// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <string>

TEST_CASE("run") {
  CHECK(emio::format(emio::runtime("{{{0}}}"), 42) == "{42}");
}
