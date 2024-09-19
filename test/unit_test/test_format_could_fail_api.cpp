// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

using namespace std::string_view_literals;

TEST_CASE("format format_args which could fail", "[format]") {
  // Test strategy:
  // * Call emio's format function with format_args as parameters.
  // Expected: format_args can be formatted or if not, the correct error is returned.

  const auto check = [](const auto& args) {
    // Check if args is a format_args.
    const emio::format_args& base_args = args;
    STATIC_CHECK(std::is_base_of_v<emio::format_args, std::decay_t<decltype(args)>>);
    STATIC_CHECK(!std::is_same_v<emio::format_args, std::decay_t<decltype(args)>>);

    CHECK(emio::formatted_size("{}", args).value() == 3);
    CHECK(emio::formatted_size("{}", base_args).value() == 3);
    CHECK(emio::formatted_size("{} {}", args, base_args).value() == 7);
    CHECK(emio::formatted_size("{}", emio::make_format_args("{}", emio::make_format_args("{}", args))).value() == 3);
    CHECK(emio::formatted_size("{}", emio::make_format_args("{}", 123)).value() == 3);
    CHECK(emio::formatted_size("{}", emio::make_format_args(emio::runtime("{}"), 123)).value() == 3);
    CHECK(emio::formatted_size("{}", emio::make_format_args(emio::runtime("{}"))) == emio::err::invalid_format);
    CHECK(emio::formatted_size("{}", emio::make_format_args(emio::runtime("{"), 123)) == emio::err::invalid_format);

    CHECK(emio::format("{}", args).value() == "123");
    CHECK(emio::format("{}", base_args).value() == "123");
    CHECK(emio::format("{} {}", args, base_args).value() == "123 123");
    CHECK(emio::format("{}", emio::make_format_args("{}", emio::make_format_args("{}", args))).value() == "123");
    CHECK(emio::format("{}", emio::make_format_args("{}", 123)).value() == "123");
    CHECK(emio::format("{}", emio::make_format_args(emio::runtime("{}"), 123)).value() == "123");
    CHECK(emio::format("{}", emio::make_format_args(emio::runtime("{}"))) == emio::err::invalid_format);
    CHECK(emio::format("{}", emio::make_format_args(emio::runtime("{"), 123)) == emio::err::invalid_format);

    CHECK(emio::print("{}", args).has_value());
    CHECK(emio::print("{}", base_args).has_value());
    CHECK(emio::print("{} {}", args, base_args).has_value());
    CHECK(emio::print("{}", emio::make_format_args("{}", emio::make_format_args("{}", args))).has_value());
    CHECK(emio::print("{}", emio::make_format_args("{}", 123)).has_value());
    CHECK(emio::print("{}", emio::make_format_args(emio::runtime("{}"), 123)).has_value());
    CHECK(emio::print("{}", emio::make_format_args(emio::runtime("{}"))) == emio::err::invalid_format);
    CHECK(emio::print("{}", emio::make_format_args(emio::runtime("{"), 123)) == emio::err::invalid_format);

    CHECK(emio::println("{}", args).has_value());
    CHECK(emio::println("{}", base_args).has_value());
    CHECK(emio::println("{} {}", args, base_args).has_value());
    CHECK(emio::println("{}", emio::make_format_args("{}", emio::make_format_args("{}", args))).has_value());
    CHECK(emio::println("{}", emio::make_format_args("{}", 123)).has_value());
    CHECK(emio::println("{}", emio::make_format_args(emio::runtime("{}"), 123)).has_value());
    CHECK(emio::println("{}", emio::make_format_args(emio::runtime("{}"))) == emio::err::invalid_format);
    CHECK(emio::println("{}", emio::make_format_args(emio::runtime("{"), 123)) == emio::err::invalid_format);

    STATIC_CHECK_FALSE(emio::format_can_fail_v<int>);
    STATIC_CHECK_FALSE(emio::format_can_fail_v<double>);
    STATIC_CHECK_FALSE(emio::format_can_fail_v<std::string>);
    STATIC_CHECK(emio::format_can_fail_v<emio::format_args>);
    STATIC_CHECK(emio::format_can_fail_v<decltype(args)>);
  };

  // Use a separate function so args lives longer.
  check(emio::make_format_args("{}", 123));
}
