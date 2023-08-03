// Unit under test.
#include <emio/scan.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

template<typename T>
struct a {

};

template<typename T>
struct b : a<T> {

};

template<typename T>
int is_arg_span2(const a<T>& t);

bool is_arg_span2(...);

template<typename T>
constexpr bool is_args_span = sizeof(is_arg_span2(std::declval<T>())) == sizeof(int);

TEST_CASE("scan", "[scan]") {
  static_assert(is_args_span<b<int>>);



    int a = 0;
    int b = 0;
    char c;
    emio::result<void> r = emio::scan("1,-2!", "{},{}{}", a, b, c);
    REQUIRE(r);
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');

    emio::reader rdr("1,-2!REST");
    r = emio::scan_from(rdr, emio::runtime("{},{}{}"), a, b, c);
    REQUIRE(r);
    CHECK(a == 1);
    CHECK(b == -2);
    CHECK(c == '!');
    CHECK(rdr.read_remaining() == "REST");
}