// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <climits>

using namespace std::string_view_literals;

// Test cases from fmt/test/format-test.cc

TEST_CASE("escape") {
  CHECK(emio::format("{{") == "{");
  CHECK(emio::format("before {{") == "before {");
  CHECK(emio::format("{{ after") == "{ after");
  CHECK(emio::format("before {{ after") == "before { after");

  CHECK(emio::format("}}") == "}");
  CHECK(emio::format("before }}") == "before }");
  CHECK(emio::format("}} after") == "} after");
  CHECK(emio::format("before }} after") == "before } after");

  CHECK(emio::format("{{}}") == "{}");
  CHECK(emio::format("{{{0}}}", 42) == "{42}");
}

TEST_CASE("unmatched_braces") {
  CHECK(emio::format(emio::runtime("{")) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("}")) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0{}")) == emio::err::invalid_format);
}

TEST_CASE("no_args") {
  CHECK(emio::format("test") == "test");
}

TEST_CASE("args_in_different_positions") {
  CHECK(emio::format("{0}", 42) == "42");
  CHECK(emio::format("before {0}", 42) == "before 42");
  CHECK(emio::format("{0} after", 42) == "42 after");
  CHECK(emio::format("before {0} after", 42) == "before 42 after");
  CHECK(emio::format("{0} = {1}", "answer", 42) == "answer = 42");
  CHECK(emio::format("{1} is the {0}", "answer", 42) == "42 is the answer");
  CHECK(emio::format("{0}{1}{0}", "abra", "cad") == "abracadabra");
}

TEST_CASE("auto_arg_index") {
  CHECK(emio::format("{}{}{}", 'a', 'b', 'c') == "abc");
  CHECK(emio::format(emio::runtime("{0}{}"), 'a', 'b') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{}{0}"), 'a', 'b') == emio::err::invalid_format);
  // CHECK(emio::format("{:.{}}", 1.2345, 2) == "1.2");
  // EXPECT_THROW_MSG((void)fmt::format(runtime("{0}:.{}"), 1.2345, 2),
  //                   format_error,
  //                   "cannot switch from manual to automatic argument indexing");
  //  EXPECT_THROW_MSG((void)fmt::format(runtime("{:.{0}}"), 1.2345, 2),
  //                   format_error,
  //                   "cannot switch from automatic to manual argument indexing");
  CHECK(emio::format(emio::runtime("{}")) == emio::err::invalid_format);
}

TEST_CASE("empty_specs") {
  CHECK(emio::format("{0:}", 42) == "42");
}

TEST_CASE("left align") {
  CHECK(emio::format("{0:<4}", 42) == "42  ");
  CHECK(emio::format("{0:<4o}", 042) == "42  ");
  CHECK(emio::format("{0:<4x}", 0x42) == "42  ");
  CHECK(emio::format("{0:<5}", -42) == "-42  ");
  CHECK(emio::format("{0:<5}", 42u) == "42   ");
  CHECK(emio::format("{0:<5}", -42l) == "-42  ");
  CHECK(emio::format("{0:<5}", 42ul) == "42   ");
  CHECK(emio::format("{0:<5}", -42ll) == "-42  ");
  CHECK(emio::format("{0:<5}", 42ull) == "42   ");
  //  CHECK(emio::format("{0:<5}", -42.0) == "-42  ");
  //  CHECK(emio::format("{0:<5}", -42.0l) == "-42  ");
  CHECK(emio::format("{0:<5}", 'c') == "c    ");
  CHECK(emio::format("{0:<5}", "abc") == "abc  ");
  CHECK(emio::format("{0:<8}", reinterpret_cast<void*>(0xface)) == "0xface  ");
}

TEST_CASE("right align") {
  CHECK(emio::format("{0:>4}", 42) == "  42");
  CHECK(emio::format("{0:>4o}", 042) == "  42");
  CHECK(emio::format("{0:>4x}", 0x42) == "  42");
  CHECK(emio::format("{0:>5}", -42) == "  -42");
  CHECK(emio::format("{0:>5}", 42u) == "   42");
  CHECK(emio::format("{0:>5}", -42l) == "  -42");
  CHECK(emio::format("{0:>5}", 42ul) == "   42");
  CHECK(emio::format("{0:>5}", -42ll) == "  -42");
  CHECK(emio::format("{0:>5}", 42ull) == "   42");
  //  CHECK(emio::format("{0:>5}", -42.0) == "  -42");
  //  CHECK(emio::format("{0:>5}", -42.0l) == "  -42");
  CHECK(emio::format("{0:>5}", 'c') == "    c");
  CHECK(emio::format("{0:>5}", "abc") == "  abc");
  CHECK(emio::format("{0:>8}", reinterpret_cast<void*>(0xface)) == "  0xface");
}

TEST_CASE("center align") {
  CHECK(emio::format("{0:^5}", 42) == " 42  ");
  CHECK(emio::format("{0:^5o}", 042) == " 42  ");
  CHECK(emio::format("{0:^5x}", 0x42) == " 42  ");
  CHECK(emio::format("{0:^5}", -42) == " -42 ");
  CHECK(emio::format("{0:^5}", 42u) == " 42  ");
  CHECK(emio::format("{0:^5}", -42l) == " -42 ");
  CHECK(emio::format("{0:^5}", 42ul) == " 42  ");
  CHECK(emio::format("{0:^5}", -42ll) == " -42 ");
  CHECK(emio::format("{0:^5}", 42ull) == " 42  ");
  //  CHECK(emio::format("{0:^5}", -42.0) == " -42 ");
  //  CHECK(emio::format("{0:^5}", -42.0l) == " -42 ");
  CHECK(emio::format("{0:^5}", 'c') == "  c  ");
  CHECK(emio::format("{0:^6}", "abc") == " abc  ");
  CHECK(emio::format("{0:^8}", reinterpret_cast<void*>(0xface)) == " 0xface ");
}

TEST_CASE("fill") {
  CHECK(emio::format("{0:*>4}", 42) == "**42");
  CHECK(emio::format("{0:*>5}", -42) == "**-42");
  CHECK(emio::format("{0:*>5}", 42u) == "***42");
  CHECK(emio::format("{0:*>5}", -42l) == "**-42");
  CHECK(emio::format("{0:*>5}", 42ul) == "***42");
  CHECK(emio::format("{0:*>5}", -42ll) == "**-42");
  CHECK(emio::format("{0:*>5}", 42ull) == "***42");
  //  CHECK(emio::format("{0:*>5}", -42.0) == "**-42");
  //  CHECK(emio::format("{0:*>5}", -42.0l) == "**-42");
  CHECK(emio::format("{0:*<5}", 'c') == "c****");
  CHECK(emio::format("{0:*<5}", "abc") == "abc**");
  CHECK(emio::format("{0:*>8}", reinterpret_cast<void*>(0xface)) == "**0xface");
  CHECK(emio::format("{:}=", "foo") == "foo=");
  CHECK(emio::format(emio::runtime(std::string_view("{:\0>4}", 6)), '*') == std::string("\0\0\0*", 4));
  //  CHECK(emio::format("{0:ж>4}", 42) == "жж42");
}

TEST_CASE("plus sign") {
  CHECK(emio::format("{0:+}", 42) == "+42");
  CHECK(emio::format("{0:+}", -42) == "-42");
  CHECK(emio::format("{0:+}", 42) == "+42");
  CHECK(emio::format(emio::runtime("{0:+}"), 42u) == emio::err::invalid_format);
  CHECK(emio::format("{0:+}", 42l) == "+42");
  CHECK(emio::format(emio::runtime("{0:+}"), 42ul) == emio::err::invalid_format);
  CHECK(emio::format("{0:+}", 42ll) == "+42");
  // #if FMT_USE_INT128
  //  CHECK(emio::format("{0:+}", __int128_t(42)) == "+42");
  // #endif
  CHECK(emio::format(emio::runtime("{0:+}"), 42ull) == emio::err::invalid_format);
  //  CHECK(emio::format("{0:+}", 42.0) == "+42");
  //  CHECK(emio::format("{0:+}", 42.0l) == "+42");
  CHECK(emio::format(emio::runtime("{0:+"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:+}"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:+}"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:+}"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("minus sign") {
  CHECK(emio::format("{0:-}", 42) == "42");
  CHECK(emio::format("{0:-}", -42) == "-42");
  CHECK(emio::format("{0:-}", 42) == "42");
  CHECK(emio::format(emio::runtime("{0:-}"), 42u) == emio::err::invalid_format);
  CHECK(emio::format("{0:-}", 42l) == "42");
  CHECK(emio::format("{0:-}", 42ll) == "42");
  //  CHECK(emio::format("{0:-}", 42.0) == "42");
  //  CHECK(emio::format("{0:-}", 42.0l) == "42");
  CHECK(emio::format(emio::runtime("{0:-}"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:-}"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:-}"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("space sign") {
  CHECK(emio::format("{0: }", 42) == " 42");
  CHECK(emio::format("{0: }", -42) == "-42");
  CHECK(emio::format("{0: }", 42) == " 42");
  CHECK(emio::format(emio::runtime("{0: }"), 42u) == emio::err::invalid_format);
  CHECK(emio::format("{0: }", 42l) == " 42");
  CHECK(emio::format(emio::runtime("{0: }"), 42ul) == emio::err::invalid_format);
  CHECK(emio::format("{0: }", 42ll) == " 42");
  CHECK(emio::format(emio::runtime("{0: }"), 42ull) == emio::err::invalid_format);
  // CHECK(emio::format("{0: }", 42.0) == " 42");
  // CHECK(emio::format("{0: }", 42.0l) == " 42");
  CHECK(emio::format(emio::runtime("{0: "), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0: }"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0: }"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0: }"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("hash flag") {
  CHECK(emio::format("{0:#}", 42) == "42");
  CHECK(emio::format("{0:#}", -42) == "-42");
  CHECK(emio::format("{0:#b}", 42) == "0b101010");
  CHECK(emio::format("{0:#B}", 42) == "0B101010");
  CHECK(emio::format("{0:#b}", -42) == "-0b101010");
  CHECK(emio::format("{0:#x}", 0x42) == "0x42");
  CHECK(emio::format("{0:#X}", 0x42) == "0X42");
  CHECK(emio::format("{0:#x}", -0x42) == "-0x42");
  CHECK(emio::format("{0:#o}", 0) == "0");
  CHECK(emio::format("{0:#o}", 042) == "042");
  CHECK(emio::format("{0:#o}", -042) == "-042");
  CHECK(emio::format("{0:#}", 42u) == "42");
  CHECK(emio::format("{0:#x}", 0x42u) == "0x42");
  CHECK(emio::format("{0:#o}", 042u) == "042");

  CHECK(emio::format("{0:#}", -42l) == "-42");
  CHECK(emio::format("{0:#x}", 0x42l) == "0x42");
  CHECK(emio::format("{0:#x}", -0x42l) == "-0x42");
  CHECK(emio::format("{0:#o}", 042l) == "042");
  CHECK(emio::format("{0:#o}", -042l) == "-042");
  CHECK(emio::format("{0:#}", 42ul) == "42");
  CHECK(emio::format("{0:#x}", 0x42ul) == "0x42");
  CHECK(emio::format("{0:#o}", 042ul) == "042");

  CHECK(emio::format("{0:#}", -42ll) == "-42");
  CHECK(emio::format("{0:#x}", 0x42ll) == "0x42");
  CHECK(emio::format("{0:#x}", -0x42ll) == "-0x42");
  CHECK(emio::format("{0:#o}", 042ll) == "042");
  CHECK(emio::format("{0:#o}", -042ll) == "-042");
  CHECK(emio::format("{0:#}", 42ull) == "42");
  CHECK(emio::format("{0:#x}", 0x42ull) == "0x42");
  CHECK(emio::format("{0:#o}", 042ull) == "042");

  //  CHECK(emio::format("{0:#}", -42.0) == "-42.0");
  //  CHECK(emio::format("{0:#}", -42.0l) == "-42.0");
  //  CHECK(emio::format("{:#.0e}", 42.0) == "4.e+01");
  //  CHECK(emio::format("{:#.0f}", 0.01) == "0.");
  //  CHECK(emio::format("{:#.2g}", 0.5) == "0.50");
  //  CHECK(emio::format("{:#.0f}", 0.5) == "0.");
  CHECK(emio::format(emio::runtime("{0:#"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:#}"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:#}"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:#}"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("zero flag") {
  //  CHECK(emio::format("{0:0}", 42) == "42");
  CHECK(emio::format("{0:05}", -42) == "-0042");
  CHECK(emio::format("{0:05}", 42u) == "00042");
  CHECK(emio::format("{0:05}", -42l) == "-0042");
  CHECK(emio::format("{0:05}", 42ul) == "00042");
  CHECK(emio::format("{0:05}", -42ll) == "-0042");
  CHECK(emio::format("{0:05}", 42ull) == "00042");
  //  CHECK(emio::format("{0:07}", -42.0) == "-000042");
  //  CHECK(emio::format("{0:07}", -42.0l) == "-000042");
  CHECK(emio::format(emio::runtime("{0:0"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:05}"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:05}"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:05}"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("width") {
  /*char format_str[buffer_size];
  safe_sprintf(format_str, "{0:%u", UINT_MAX);
  increment(format_str + 3);
  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0), format_error,
                   "number is too big");
  size_t size = std::strlen(format_str);
  format_str[size] = '}';
  format_str[size + 1] = 0;
  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0), format_error,
                   "number is too big");

  safe_sprintf(format_str, "{0:%u", INT_MAX + 1u);
  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0), format_error,
                   "number is too big");
  safe_sprintf(format_str, "{0:%u}", INT_MAX + 1u);
  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0), format_error,
                   "number is too big");*/
  CHECK(emio::format("{0:4}", -42) == " -42");
  CHECK(emio::format("{0:5}", 42u) == "   42");
  CHECK(emio::format("{0:6}", -42l) == "   -42");
  CHECK(emio::format("{0:7}", 42ul) == "     42");
  CHECK(emio::format("{0:6}", -42ll) == "   -42");
  CHECK(emio::format("{0:7}", 42ull) == "     42");
  //  CHECK(emio::format("{0:8}", -1.23) == "   -1.23");
  //  CHECK(emio::format("{0:9}", -1.23l) == "    -1.23");
  CHECK(emio::format("{0:10}", reinterpret_cast<void*>(0xcafe)) == "    0xcafe");
  CHECK(emio::format("{0:11}", 'x') == "x          ");
  CHECK(emio::format("{0:12}", "str") == "str         ");
  //  EXPECT_EQ(fmt::format("{:*^6}", "🤡"), "**🤡**");
  //  EXPECT_EQ(fmt::format("{:*^8}", "你好"), "**你好**");
  // EXPECT_EQ(fmt::format("{:#6}", 42.0), "  42.0");
  CHECK(emio::format("{:6c}", static_cast<int>('x')) == "x     ");
  //  CHECK(emio::format("{:>06.0f}", 0.00884311) == "000000");
}

TEST_CASE("format bool") {
  CHECK(emio::format("{}", true) == "true");
  CHECK(emio::format("{}", false) == "false");
  CHECK(emio::format("{:d}", true) == "1");
  CHECK(emio::format("{:5}", true) == "true ");
  CHECK(emio::format("{:s}", true) == "true");
  CHECK(emio::format("{:s}", false) == "false");
  CHECK(emio::format("{:6s}", false) == "false ");
}

TEST_CASE("format short") {
  short s = 42;
  CHECK(emio::format("{0:d}", s) == "42");
  unsigned short us = 42;
  CHECK(emio::format("{0:d}", us) == "42");
}

TEST_CASE("format int") {
  CHECK(emio::format(emio::runtime("{0:v"), 42) == emio::err::invalid_format);
  // check_unknown_types(42, "bBdoxXnLc", "integer");
  CHECK(emio::format("{:c}", static_cast<int>('x')) == "x");
}

TEST_CASE("format bin") {
  CHECK(emio::format("{0:b}", 0) == "0");
  CHECK(emio::format("{0:b}", 42) == "101010");
  CHECK(emio::format("{0:b}", 42u) == "101010");
  CHECK(emio::format("{0:b}", -42) == "-101010");
  CHECK(emio::format("{0:b}", 12345) == "11000000111001");
  CHECK(emio::format("{0:b}", 0x12345678) == "10010001101000101011001111000");
  CHECK(emio::format("{0:b}", 0x90ABCDEF) == "10010000101010111100110111101111");
  CHECK(emio::format("{0:b}", std::numeric_limits<uint32_t>::max()) == "11111111111111111111111111111111");
}

TEST_CASE("format dec") {
  CHECK(emio::format("{0}", 0) == "0");
  CHECK(emio::format("{0}", 42) == "42");
  CHECK(emio::format("{:}>", 42) == "42>");
  CHECK(emio::format("{0:d}", 42) == "42");
  CHECK(emio::format("{0}", 42u) == "42");
  CHECK(emio::format("{0}", -42) == "-42");
  CHECK(emio::format("{0}", 12345) == "12345");
  CHECK(emio::format("{0}", 67890) == "67890");
  //  #if FMT_USE_INT128
  //  CHECK(emio::format("{0}", static_cast<__int128_t>(0)) == "0");
  //  CHECK(emio::format("{0}", static_cast<__uint128_t>(0)) == "0");
  //  CHECK(emio::format("{0}", static_cast<__int128_t>(INT64_MAX) + 1) == "9223372036854775808");
  //  CHECK(emio::format("{0}", static_cast<__int128_t>(INT64_MIN) - 1) == "-9223372036854775809");
  //  CHECK(emio::format("{0}", static_cast<__int128_t>(UINT64_MAX) + 1) == "18446744073709551616");
  //  CHECK(emio::format("{0}", int128_max) == "170141183460469231731687303715884105727");
  //  CHECK(emio::format("{0}", int128_min) == "-170141183460469231731687303715884105728");
  //  CHECK(emio::format("{0}", uint128_max) == "340282366920938463463374607431768211455");
  //  #endif

  /*char buffer[buffer_size];
  safe_sprintf(buffer, "%d", INT_MIN);
  EXPECT_EQ(buffer, fmt::format("{0}", INT_MIN));
  safe_sprintf(buffer, "%d", INT_MAX);
  EXPECT_EQ(buffer, fmt::format("{0}", INT_MAX));
  safe_sprintf(buffer, "%u", UINT_MAX);
  EXPECT_EQ(buffer, fmt::format("{0}", UINT_MAX));
  safe_sprintf(buffer, "%ld", 0 - static_cast<unsigned long>(LONG_MIN));
  EXPECT_EQ(buffer, fmt::format("{0}", LONG_MIN));
  safe_sprintf(buffer, "%ld", LONG_MAX);
  EXPECT_EQ(buffer, fmt::format("{0}", LONG_MAX));
  safe_sprintf(buffer, "%lu", ULONG_MAX);
  EXPECT_EQ(buffer, fmt::format("{0}", ULONG_MAX));*/
}

TEST_CASE("format hex") {
  CHECK(emio::format("{0:x}", 0) == "0");
  CHECK(emio::format("{0:x}", 0x42) == "42");
  CHECK(emio::format("{0:x}", 0x42u) == "42");
  CHECK(emio::format("{0:x}", -0x42) == "-42");
  CHECK(emio::format("{0:x}", 0x12345678) == "12345678");
  CHECK(emio::format("{0:x}", 0x90abcdef) == "90abcdef");
  CHECK(emio::format("{0:X}", 0x12345678) == "12345678");
  CHECK(emio::format("{0:X}", 0x90ABCDEF) == "90ABCDEF");
  //  #if FMT_USE_INT128
  //  CHECK(emio::format("{0:x}", static_cast<__int128_t>(0)) == "0");
  //  CHECK(emio::format("{0:x}", static_cast<__uint128_t>(0)) == "0");
  //  CHECK(emio::format("{0:x}", static_cast<__int128_t>(INT64_MAX) + 1) == "8000000000000000");
  //  CHECK(emio::format("{0:x}", static_cast<__int128_t>(INT64_MIN) - 1) == "-8000000000000001");
  //  CHECK(emio::format("{0:x}", static_cast<__int128_t>(UINT64_MAX) + 1) == "10000000000000000");
  //  CHECK(emio::format("{0:x}", int128_max) == "7fffffffffffffffffffffffffffffff");
  //  CHECK(emio::format("{0:x}", int128_min) == "-80000000000000000000000000000000");
  //  CHECK(emio::format("{0:x}", uint128_max) == "ffffffffffffffffffffffffffffffff");
  //  #endif

  /*char buffer[buffer_size];
  safe_sprintf(buffer, "-%x", 0 - static_cast<unsigned>(INT_MIN));
  EXPECT_EQ(buffer, fmt::format("{0:x}", INT_MIN));
  safe_sprintf(buffer, "%x", INT_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:x}", INT_MAX));
  safe_sprintf(buffer, "%x", UINT_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:x}", UINT_MAX));
  safe_sprintf(buffer, "-%lx", 0 - static_cast<unsigned long>(LONG_MIN));
  EXPECT_EQ(buffer, fmt::format("{0:x}", LONG_MIN));
  safe_sprintf(buffer, "%lx", LONG_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:x}", LONG_MAX));
  safe_sprintf(buffer, "%lx", ULONG_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:x}", ULONG_MAX));*/
}

TEST_CASE("format_oct") {
  CHECK(emio::format("{0:o}", 0) == "0");
  CHECK(emio::format("{0:o}", 042) == "42");
  CHECK(emio::format("{0:o}", 042u) == "42");
  CHECK(emio::format("{0:o}", -042) == "-42");
  CHECK(emio::format("{0:o}", 012345670) == "12345670");
  /*#if FMT_USE_INT128
    CHECK(emio::format("{0:o}", static_cast<__int128_t>(0)) == "0");
    CHECK(emio::format("{0:o}", static_cast<__uint128_t>(0)) == "0");
    CHECK(emio::format("{0:o}", static_cast<__int128_t>(INT64_MAX) + 1) == "1000000000000000000000");
    CHECK(emio::format("{0:o}", static_cast<__int128_t>(INT64_MIN) - 1) == "-1000000000000000000001");
    CHECK(emio::format("{0:o}", static_cast<__int128_t>(UINT64_MAX) + 1) == "2000000000000000000000");
    CHECK(emio::format("{0:o}", int128_max) == "1777777777777777777777777777777777777777777");
    CHECK(emio::format("{0:o}", int128_min) == "-2000000000000000000000000000000000000000000");
    CHECK(emio::format("{0:o}", uint128_max) == "3777777777777777777777777777777777777777777");
  #endif*/

  /*char buffer[buffer_size];
  safe_sprintf(buffer, "-%o", 0 - static_cast<unsigned>(INT_MIN));
  EXPECT_EQ(buffer, fmt::format("{0:o}", INT_MIN));
  safe_sprintf(buffer, "%o", INT_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:o}", INT_MAX));
  safe_sprintf(buffer, "%o", UINT_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:o}", UINT_MAX));
  safe_sprintf(buffer, "-%lo", 0 - static_cast<unsigned long>(LONG_MIN));
  EXPECT_EQ(buffer, fmt::format("{0:o}", LONG_MIN));
  safe_sprintf(buffer, "%lo", LONG_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:o}", LONG_MAX));
  safe_sprintf(buffer, "%lo", ULONG_MAX);
  EXPECT_EQ(buffer, fmt::format("{0:o}", ULONG_MAX));*/
}

TEST_CASE("format char") {
  //  const char types[] = "cbBdoxX";
  //  check_unknown_types('a', types, "char");
  CHECK(emio::format("{0}", 'a') == "a");
  CHECK(emio::format("{0:c}", 'z') == "z");
  //  int n = 'x';
  //  for (const char* type = types + 1; *type; ++type) {
  //    std::string format_str = fmt::format("{{:{}}}", *type);
  //    EXPECT_EQ(fmt::format(runtime(format_str), n),
  //              fmt::format(runtime(format_str), 'x'))
  //        << format_str;
  //  }
  //  EXPECT_EQ(fmt::format("{:02X}", n), fmt::format("{:02X}", 'x'));

  CHECK(emio::format("{}", '\n') == "\n");
  CHECK(emio::format("{:?}", '\n') == "'\\n'");
}

TEST_CASE("format_volatile_char") {
  //  volatile char c = 'x';
  // TODO: CHECK(emio::format("{}", c) == "x");
}

TEST_CASE("format_unsigned_char") {
  CHECK(emio::format("{}", static_cast<unsigned char>(42)) == "42");
  CHECK(emio::format("{}", static_cast<uint8_t>(42)) == "42");
}

TEST_CASE("format_cstring") {
  // check_unknown_types("test", "sp", "string");
  CHECK(emio::format("{0}", "test") == "test");
  CHECK(emio::format("{0:s}", "test") == "test");
  char nonconst[] = "nonconst";
  CHECK(emio::format("{0}", nonconst) == "nonconst");
  //  EXPECT_THROW_MSG(
  //      (void)fmt::format(runtime("{0}"), static_cast<const char*>(nullptr)),
  //      format_error, "string pointer is null");
}

void function_pointer_test(int, double, std::string) {}

TEST_CASE("format_pointer") {
  // check_unknown_types(reinterpret_cast<void*>(0x1234), "p", "pointer");
  CHECK(emio::format("{0}", static_cast<void*>(nullptr)) == "0x0");
  CHECK(emio::format("{0}", reinterpret_cast<void*>(0x1234)) == "0x1234");
  CHECK(emio::format("{0:p}", reinterpret_cast<void*>(0x1234)) == "0x1234");
  CHECK(emio::format("{0}", reinterpret_cast<void*>(~uintptr_t())) ==
        ("0x" + std::string(sizeof(void*) * CHAR_BIT / 4, 'f')));
  // CHECK(emio::format("{}", fmt::ptr(reinterpret_cast<int*>(0x1234))) == "0x1234");

  // std::unique_ptr<int> up(new int(1));
  // CHECK(emio::format("{}", fmt::ptr(up.get())), fmt::format("{}", fmt::ptr(up)));
  // std::shared_ptr<int> sp(new int(1));
  // EXPECT_EQ(fmt::format("{}", fmt::ptr(sp.get())), fmt::format("{}", fmt::ptr(sp)));
  // EXPECT_EQ(fmt::format("{}", fmt::detail::bit_cast<const void*>(&function_pointer_test)), fmt::format("{}",
  // fmt::ptr(function_pointer_test)));
  CHECK(emio::format("{}", nullptr) == "0x0");
}

enum color { red, green, blue };

TEST_CASE("format_enum") {
  CHECK(emio::format("{}", static_cast<std::underlying_type_t<color>>(color::red)) == "0");
  CHECK(emio::format("{}", color::red) == "0");
}

TEST_CASE("format_string") {
  CHECK(emio::format("{0}", std::string("test")) == "test");
  CHECK(emio::format("{0}", std::string("test")) == "test");
  CHECK(emio::format("{:?}", std::string("test")) == "\"test\"");
  CHECK(emio::format("{:*^10?}", std::string("test")) == "**\"test\"**");
  CHECK(emio::format("{:?}", std::string("\test")) == "\"\\test\"");

  CHECK(emio::format(emio::runtime("{:x}"), std::string("test")) == emio::err::invalid_format);
}

TEST_CASE("format_string_view") {
  CHECK(emio::format("{}", std::string_view("test")) == "test");
  CHECK(emio::format("{:?}", std::string_view("t\nst")) == "\"t\\nst\"");
  CHECK(emio::format("{}", std::string_view()) == "");
  CHECK(emio::format("{:?}", std::string_view("t\n\r\t\\\'\"st")) == "\"t\\n\\r\\t\\\\\\'\\\"st\"");
  CHECK(emio::format("{:?}", std::string_view("\x05\xab\xEf")) == "\"\\x05\\xAB\\xEF\"");
}

#ifdef FMT_USE_STRING_VIEW
struct string_viewable {};

FMT_BEGIN_NAMESPACE
template <>
struct formatter<string_viewable> : formatter<std::string_view> {
  auto format(string_viewable, format_context& ctx) -> decltype(ctx.out()) {
    return formatter<std::string_view>::format("foo", ctx);
  }
};
FMT_END_NAMESPACE

TEST(format_test, format_std_string_view) {
  EXPECT_EQ("test", fmt::format("{}", std::string_view("test")));
  EXPECT_EQ("foo", fmt::format("{}", string_viewable()));
}

struct explicitly_convertible_to_std_string_view {
  explicit operator std::string_view() const {
    return "foo";
  }
};

template <>
struct fmt::formatter<explicitly_convertible_to_std_string_view> : formatter<std::string_view> {
  auto format(explicitly_convertible_to_std_string_view v, format_context& ctx) -> decltype(ctx.out()) {
    return format_to(ctx.out(), "'{}'", std::string_view(v));
  }
};

TEST(format_test, format_explicitly_convertible_to_std_string_view) {
  EXPECT_EQ("'foo'", fmt::format("{}", explicitly_convertible_to_std_string_view()));
}
#endif

TEST_CASE("format_str") {
  using emio::detail::format::validate_format_string;

  CHECK(validate_format_string("abc"sv));
  CHECK(!validate_format_string("abc{"sv));
  CHECK(!validate_format_string("abc}"sv));
  CHECK(validate_format_string<int>("abc{}"sv));
  CHECK(validate_format_string<int>("abc{0}"sv));
  CHECK(!validate_format_string<int>("abc{1}"sv));
  CHECK(!validate_format_string<int, short>("abc{0}{}"sv));
  CHECK(!validate_format_string<int, short>("abc{}{1}"sv));
  CHECK(validate_format_string<int, short>("abc{0}{1}"sv));
  CHECK(validate_format_string<int>("abc{:}"sv));
  CHECK(!validate_format_string<int>("abc{:f}"sv));
  CHECK(!validate_format_string<int>("abc"sv));

  CHECK(validate_format_string<int>("{:<10}"sv));
  CHECK(validate_format_string<int>("{:^10}"sv));
  CHECK(validate_format_string<int>("{:>10}"sv));
  CHECK(validate_format_string<int>("{:<<10}"sv));
  CHECK(validate_format_string<int>("{:^^10}"sv));
  CHECK(validate_format_string<int>("{:>>10}"sv));
  CHECK(!validate_format_string<int>("{:<}"sv));

  CHECK(validate_format_string<int>("{:a<10}"sv));
  CHECK(validate_format_string<int>("{:a^10}"sv));
  CHECK(validate_format_string<int>("{:a>10}"sv));
  CHECK(!validate_format_string<int>("{:a=10}"sv));
  CHECK(!validate_format_string<int>("{:a<}"sv));
  CHECK(!validate_format_string<int>("{:a<010}"sv));

  CHECK(validate_format_string<int>("{:a<-10}"sv));
  CHECK(validate_format_string<int>("{:a<+10}"sv));
  CHECK(validate_format_string<int>("{:a< 10}"sv));
  CHECK(validate_format_string<int>("{:a<-#10}"sv));
  CHECK(!validate_format_string<int>("{:a<--10}"sv));

  CHECK(validate_format_string<int>("{:+}"sv));
  CHECK(validate_format_string<int>("{:-}"sv));
  CHECK(validate_format_string<int>("{: }"sv));
  CHECK(!validate_format_string<int>("{:<}"sv));

  CHECK(validate_format_string<int>("{:+10}"sv));
  CHECK(validate_format_string<int>("{:-10}"sv));
  CHECK(validate_format_string<int>("{: 10}"sv));

  CHECK(validate_format_string<int>("{:+#10}"sv));
  CHECK(validate_format_string<int>("{:-#10}"sv));
  CHECK(validate_format_string<int>("{: #10}"sv));

  CHECK(validate_format_string<int>("{:010}"sv));
  CHECK(validate_format_string<int>("{:-010}"sv));
  CHECK(validate_format_string<int>("{:-#010}"sv));
  CHECK(validate_format_string<int>("{:0010}"sv));
  CHECK(!validate_format_string<int>("{:0}"sv));
  CHECK(!validate_format_string<int>("{:--010}"sv));
}
