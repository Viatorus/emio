// Unit under test.
#include <emio/format.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>
#include <climits>
#include <cmath>

using namespace std::string_view_literals;

// Test cases from fmt/test/format-test.cc - 9.1.0

namespace {

template <typename... Args>
bool validate_format_string(std::string_view str) {
  return emio::detail::format::format_trait::validate_string<Args...>(str);
}

}  // namespace

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
  CHECK(emio::format("{:.2}", 1.2345) == "1.2");
  CHECK(emio::format(emio::runtime("{0}:.{}"), 1.2345, 2) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{:.{0}}"), 1.2345, 2) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{}")) == emio::err::invalid_format);
}

TEST_CASE("empty_specs") {
  CHECK(emio::format("{0:}", 42) == "42");
}

TEST_CASE("left align") {
  CHECK(emio::format("{0:<}", 42) == "42");
  CHECK(emio::format("{0:<4}", 42) == "42  ");
  CHECK(emio::format("{0:<4o}", 042) == "42  ");
  CHECK(emio::format("{0:<4x}", 0x42) == "42  ");
  CHECK(emio::format("{0:<5}", -42) == "-42  ");
  CHECK(emio::format("{0:<5}", 42u) == "42   ");
  CHECK(emio::format("{0:<5}", -42l) == "-42  ");
  CHECK(emio::format("{0:<5}", 42ul) == "42   ");
  CHECK(emio::format("{0:<5}", -42ll) == "-42  ");
  CHECK(emio::format("{0:<5}", 42ull) == "42   ");
  CHECK(emio::format("{0:<5}", -42.0) == ("-42  "));
  CHECK(emio::format("{0:<5}", 'c') == "c    ");
  CHECK(emio::format("{0:<5}", "abc") == "abc  ");
  CHECK(emio::format("{0:<8}", reinterpret_cast<void*>(0xface)) == "0xface  ");
}

TEST_CASE("right align") {
  CHECK(emio::format("{0:>}", 42) == "42");
  CHECK(emio::format("{0:>4}", 42) == "  42");
  CHECK(emio::format("{0:>4o}", 042) == "  42");
  CHECK(emio::format("{0:>4x}", 0x42) == "  42");
  CHECK(emio::format("{0:>5}", -42) == "  -42");
  CHECK(emio::format("{0:>5}", 42u) == "   42");
  CHECK(emio::format("{0:>5}", -42l) == "  -42");
  CHECK(emio::format("{0:>5}", 42ul) == "   42");
  CHECK(emio::format("{0:>5}", -42ll) == "  -42");
  CHECK(emio::format("{0:>5}", 42ull) == "   42");
  CHECK(emio::format("{0:>5}", -42.0) == "  -42");
  CHECK(emio::format("{0:>5}", 'c') == "    c");
  CHECK(emio::format("{0:>5}", "abc") == "  abc");
  CHECK(emio::format("{0:>8}", reinterpret_cast<void*>(0xface)) == "  0xface");
}

TEST_CASE("center align") {
  CHECK(emio::format("{0:^}", 42) == "42");
  CHECK(emio::format("{0:^5}", 42) == " 42  ");
  CHECK(emio::format("{0:^5o}", 042) == " 42  ");
  CHECK(emio::format("{0:^5x}", 0x42) == " 42  ");
  CHECK(emio::format("{0:^5}", -42) == " -42 ");
  CHECK(emio::format("{0:^5}", 42u) == " 42  ");
  CHECK(emio::format("{0:^5}", -42l) == " -42 ");
  CHECK(emio::format("{0:^5}", 42ul) == " 42  ");
  CHECK(emio::format("{0:^5}", -42ll) == " -42 ");
  CHECK(emio::format("{0:^5}", 42ull) == " 42  ");
  CHECK(emio::format("{0:^5}", -42.0) == " -42 ");
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
  CHECK(emio::format("{0:*>5}", -42.0) == "**-42");
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
  CHECK(emio::format("{0:+}", 42.0) == "+42");
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
  CHECK(emio::format("{0:-}", 42.0) == "42");
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
  CHECK(emio::format("{0: }", 42.0) == " 42");
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

  CHECK(emio::format("{0:#}", -42.0) == "-42.");
  CHECK(emio::format("{:#.0e}", 42.0) == "4.e+01");
  CHECK(emio::format("{:#.0f}", 0.01) == "0.");
  CHECK(emio::format("{:#.2g}", 0.5) == "0.50");
  CHECK(emio::format("{:#.0f}", 0.5) == "0.");
  CHECK(emio::format(emio::runtime("{0:#"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:#}"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:#}"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:#}"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("zero flag") {
  CHECK(emio::format(emio::runtime("{0:0}")) == emio::err::invalid_format);  // Zero without width doesn't make sense.
  CHECK(emio::format("{0:05}", -42) == "-0042");
  CHECK(emio::format("{0:05}", 42u) == "00042");
  CHECK(emio::format("{0:05}", -42l) == "-0042");
  CHECK(emio::format("{0:05}", 42ul) == "00042");
  CHECK(emio::format("{0:05}", -42ll) == "-0042");
  CHECK(emio::format("{0:05}", 42ull) == "00042");
  CHECK(emio::format("{0:07}", -42.0) == "-000042");
  CHECK(emio::format(emio::runtime("{0:0"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:05}"), 'c') == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:05}"), "abc") == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:05}"), reinterpret_cast<void*>(0x42)) == emio::err::invalid_format);
}

TEST_CASE("zero_flag_and_align") {
  // If the 0 character and an align option both appear, the 0 character is ignored.
  CHECK(emio::format("{0:<05}", 42) == "42   ");
  CHECK(emio::format("{0:<05}", -42) == "-42  ");
  CHECK(emio::format("{0:^05}", 42) == " 42  ");
  CHECK(emio::format("{0:^05}", -42) == " -42 ");
  CHECK(emio::format("{0:>05}", 42) == "   42");
  CHECK(emio::format("{0:>05}", -42) == "  -42");
  CHECK(emio::format("{0:>05}", -42) == "  -42");
}

TEST_CASE("zero_flag_sign_and_prefix") {
  CHECK(emio::format("{0:#8x}", 42) == "    0x2a");
  CHECK(emio::format("{0:+#8x}", 42) == "   +0x2a");
  CHECK(emio::format("{0:#08x}", 42) == "0x00002a");
  CHECK(emio::format("{0:+#08x}", 42) == "+0x0002a");
  CHECK(emio::format("{0:#8x}", -42) == "   -0x2a");
  CHECK(emio::format("{0:#08x}", -42) == "-0x0002a");
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
  CHECK(emio::format("{0:8}", -1.23) == "   -1.23");
  CHECK(emio::format("{0:10}", reinterpret_cast<void*>(0xcafe)) == "    0xcafe");
  CHECK(emio::format("{0:11}", 'x') == "x          ");
  CHECK(emio::format("{0:12}", "str") == "str         ");
  //  EXPECT_EQ(fmt::format("{:*^6}", "🤡"), "**🤡**");
  //  EXPECT_EQ(fmt::format("{:*^8}", "你好"), "**你好**");
  CHECK(emio::format("{:#6}", 42.) == "   42.");
  CHECK(emio::format("{:6c}", static_cast<int>('x')) == "x     ");
  CHECK(emio::format("{:06.0f}", 0.00884311) == "000000");
  CHECK(emio::format("{:>06.0f}", 0.00884311) == "     0");
  CHECK(emio::format("{:5?}", "\n") == "\"\\n\" ");
}

TEST_CASE("precision") {
  //  char format_str[buffer_size];
  ////  safe_sprintf(format_str, "{0:.%u", UINT_MAX);
  ////  increment(format_str + 4);
  ////  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0.0), format_error,
  ////                   "number is too big");
  ////  size_t size = std::strlen(format_str);
  ////  format_str[size] = '}';
  ////  format_str[size + 1] = 0;
  ////  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0.0), format_error,
  ////                   "number is too big");
  ////
  ////  safe_sprintf(format_str, "{0:.%u", INT_MAX + 1u);
  ////  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0.0), format_error,
  ////                   "number is too big");
  ////  safe_sprintf(format_str, "{0:.%u}", INT_MAX + 1u);
  ////  EXPECT_THROW_MSG((void)fmt::format(runtime(format_str), 0.0), format_error,
  ////                   "number is too big");

  CHECK(emio::format(emio::runtime("{0:."), 0.0) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.}"), 0.0) == emio::err::invalid_format);

  CHECK(emio::format(emio::runtime("{0:.2"), 0) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2}"), 42) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), 42) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2}"), 42u) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), 42u) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2}"), 42l) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), 42l) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2}"), 42ul) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), 42ul) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2}"), 42ll) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), 42ll) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2}"), 42ull) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), 42ull) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:3.0}"), 'x') == emio::err::invalid_format);
  CHECK(emio::format("{0:.2}", 1.2345) == "1.2");
  CHECK(emio::format("{:.2}", 1.234e56) == "1.2e+56");
  CHECK(emio::format("{0:.3}", 1.1) == "1.1");
  CHECK(emio::format("{:.0e}", 1.0) == "1e+00");
  CHECK(emio::format("{:9.1e}", 0.0) == "  0.0e+00");
  CHECK(emio::format("{:.494}", 4.9406564584124654E-324) ==
        "4.9406564584124654417656879286822137236505980261432476442558568250067550"
        "727020875186529983636163599237979656469544571773092665671035593979639877"
        "479601078187812630071319031140452784581716784898210368871863605699873072"
        "305000638740915356498438731247339727316961514003171538539807412623856559"
        "117102665855668676818703956031062493194527159149245532930545654440112748"
        "012970999954193198940908041656332452475714786901472678015935523861155013"
        "480352649347201937902681071074917033322268447533357208324319361e-324");
  CHECK(emio::format("{:.1074f}", 1.1125369292536e-308) ==
        "0.0000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000000000000000000000000000000000000000000000000000000"
        "000000000000000000000111253692925360019747947051741965785554081512200979"
        "355021686109411883779182127659725163430929750364498219730822952552570601"
        "152163505899912777129583674906301179059298598412303893909188340988729019"
        "014361467448914817838555156840459458527907308695109202499990850735085304"
        "478476991912072201449236975063640913461919914396877093174125167509869762"
        "482369631100360266123742648159508919592746619553246586039571522788247697"
        "156360766271842991667238355464496455107749716934387136380536472531224398"
        "559833794807213172371254492216255558078524900147957309382830827524104234"
        "530961756787819847850302379672357738807808384667004752163416921762619527"
        "462847642037420991432005657440259928195996762610375541867198059294212446"
        "81962777939941034720757232455434770912461317493580281734466552734375");

  std::string outputs[] = {
      "-0X1.41FE3FFE71C9E000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000P+127",
      "-0XA.0FF1FFF38E4F0000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000000000000000000000000"
      "000000000000000000000000000000000000000000000000000P+124"};
  //  EXPECT_THAT(outputs,
  //              testing::Contains(fmt::format("{:.838A}", -2.14001164E+38)));

  //  if (std::numeric_limits<long double>::digits == 64) {
  //    auto ld = (std::numeric_limits<long double>::min)();
  //    EXPECT_EQ(fmt::format("{:.0}", ld), "3e-4932");
  //    EXPECT_EQ(
  //        fmt::format("{:0g}", std::numeric_limits<long double>::denorm_min()),
  //        "3.6452e-4951");
  //  }

  CHECK(emio::format("{:#.0f}", 123.0) == "123.");
  CHECK(emio::format("{:.02f}", 1.234) == "1.23");
  CHECK(emio::format("{:.1g}", 0.001) == "0.001");
  CHECK(emio::format("{}", 1019666432.0f) == "1019666432");  // Different from FMT: 1019666400 but not totally wrong.
  CHECK(emio::format("{:.0e}", 9.5) == "1e+01");
  CHECK(emio::format("{:.1e}", 1e-34) == "1.0e-34");

  CHECK(emio::format(emio::runtime("{0:.2}"), reinterpret_cast<void*>(0xcafe)) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2f}"), reinterpret_cast<void*>(0xcafe)) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.2e}"), reinterpret_cast<void*>(0xcafe)) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.-1e}"), 42.0) == emio::err::invalid_format);
  CHECK(emio::format(emio::runtime("{0:.-1e}"), 42.0) == emio::err::invalid_format);

  //  CHECK(emio::format("{0:.2}", "str") == "st");
  //  CHECK(emio::format("{0:.5}", "вожыкі") == "вожык");
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
  CHECK(emio::format("{0}", INT_MIN) == buffer);
  safe_sprintf(buffer, "%d", INT_MAX);
  CHECK(emio::format("{0}", INT_MAX) == buffer);
  safe_sprintf(buffer, "%u", UINT_MAX);
  CHECK(emio::format("{0}", UINT_MAX) == buffer);
  safe_sprintf(buffer, "%ld", 0 - static_cast<unsigned long>(LONG_MIN));
  CHECK(emio::format("{0}", LONG_MIN) == buffer);
  safe_sprintf(buffer, "%ld", LONG_MAX);
  CHECK(emio::format("{0}", LONG_MAX) == buffer);
  safe_sprintf(buffer, "%lu", ULONG_MAX);
  CHECK(emio::format("{0}", ULONG_MAX) == buffer);*/
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
  CHECK(emio::format("{0:x}", INT_MIN) == buffer);
  safe_sprintf(buffer, "%x", INT_MAX);
  CHECK(emio::format("{0:x}", INT_MAX) == buffer);
  safe_sprintf(buffer, "%x", UINT_MAX);
  CHECK(emio::format("{0:x}", UINT_MAX) == buffer);
  safe_sprintf(buffer, "-%lx", 0 - static_cast<unsigned long>(LONG_MIN));
  CHECK(emio::format("{0:x}", LONG_MIN) == buffer);
  safe_sprintf(buffer, "%lx", LONG_MAX);
  CHECK(emio::format("{0:x}", LONG_MAX) == buffer);
  safe_sprintf(buffer, "%lx", ULONG_MAX);
  CHECK(emio::format("{0:x}", ULONG_MAX) == buffer);*/
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
  CHECK(emio::format("{0:o}", INT_MIN) == buffer);
  safe_sprintf(buffer, "%o", INT_MAX);
  CHECK(emio::format("{0:o}", INT_MAX) == buffer);
  safe_sprintf(buffer, "%o", UINT_MAX);
  CHECK(emio::format("{0:o}", UINT_MAX) == buffer);
  safe_sprintf(buffer, "-%lo", 0 - static_cast<unsigned long>(LONG_MIN));
  CHECK(emio::format("{0:o}", LONG_MIN) == buffer);
  safe_sprintf(buffer, "%lo", LONG_MAX);
  CHECK(emio::format("{0:o}", LONG_MAX) == buffer);
  safe_sprintf(buffer, "%lo", ULONG_MAX);
  CHECK(emio::format("{0:o}", ULONG_MAX) == buffer);*/
}

TEST_CASE("format_double") {
  CHECK(emio::format("{}", 0.0) == "0");
  //  check_unknown_types(1.2, "eEfFgGaAnL%", "double");
  CHECK(emio::format("{:}", 0.0) == "0");
  CHECK(emio::format("{:f}", 0.0) == "0.000000");
  CHECK(emio::format("{:g}", 0.0) == "0");
  CHECK(emio::format("{:}", 392.65) == "392.65");
  CHECK(emio::format("{:g}", 392.65) == "392.65");
  CHECK(emio::format("{:G}", 392.65) == "392.65");
  CHECK(emio::format("{:g}", 4.9014e6) == "4.9014e+06");
  CHECK(emio::format("{:f}", 392.65) == "392.650000");
  CHECK(emio::format("{:F}", 392.65) == "392.650000");
  //  CHECK(emio::format("{:L}", 42.0) == "42");
  //  CHECK(emio::format("{:24a}", 4.2f) == "           0x1.0cccccp+2");
  //  CHECK(emio::format("{:24a}", 4.2) == "    0x1.0cccccccccccdp+2");
  //  CHECK(emio::format("{:<24a}", 4.2) == "0x1.0cccccccccccdp+2    ");
  CHECK(emio::format("{0:e}", 392.65) == "3.926500e+02");
  CHECK(emio::format("{0:E}", 392.65) == "3.926500E+02");
  CHECK(emio::format("{0:+010.4g}", 392.65) == "+0000392.6");

  //  char buffer[buffer_size];
  //  double xd = 0x1.ffffffffffp+2;
  //  safe_sprintf(buffer, "%.*a", 10, xd);
  //  CHECK(emio::format("{:.10a}", xd) == buffer);
  //  safe_sprintf(buffer, "%.*a", 9, xd);
  //  CHECK(emio::format("{:.9a}", xd) == buffer);

  //  if (std::numeric_limits<long double>::digits == 64) {
  //    auto ld = 0xf.ffffffffffp-3l;
  //    safe_sprintf(buffer, "%La", ld);
  //    CHECK(emio::format("{:a}", ld) == buffer);
  //    safe_sprintf(buffer, "%.*La", 10, ld);
  //    CHECK(emio::format("{:.10a}", ld) == buffer);
  //    safe_sprintf(buffer, "%.*La", 9, ld);
  //    CHECK(emio::format("{:.9a}", ld) == buffer);
  //  }

  //  if (fmt::detail::const_check(std::numeric_limits<double>::is_iec559)) {
  //    double d = (std::numeric_limits<double>::min)();
  //    CHECK(emio::format("{:a}", d) == "0x1p-1022");
  //    CHECK(emio::format("{:#a}", d) == "0x1.p-1022");
  //
  //    d = (std::numeric_limits<double>::max)();
  //    safe_sprintf(buffer, "%a", d);
  //    CHECK(emio::format("{:a}", d) == buffer);
  //
  //    d = std::numeric_limits<double>::denorm_min();
  //    CHECK(emio::format("{:a}", d) == "0x0.0000000000001p-1022");
  //  }

  //  safe_sprintf(buffer, "%.*a", 10, 4.2);
  //  CHECK(emio::format("{:.10a}", 4.2) == buffer);
  //
  //  CHECK(emio::format("{:a}", -42.0) == "-0x1.5p+5");
  //  CHECK(emio::format("{:A}", -42.0) == "-0X1.5P+5");

  CHECK(emio::format("{:f}", 9223372036854775807.0) == "9223372036854775808.000000");
}

TEST_CASE("precision_rounding") {
  CHECK(emio::format("{:.0f}", 0.0) == "0");
  CHECK(emio::format("{:.0f}", 0.01) == "0");
  CHECK(emio::format("{:.0f}", 0.1) == "0");
  CHECK(emio::format("{:.3f}", 0.00049) == "0.000");
  CHECK(emio::format("{:.3f}", 0.0005) == "0.001");
  CHECK(emio::format("{:.3f}", 0.00149) == "0.001");
  CHECK(emio::format("{:.3f}", 0.0015) == "0.002");
  CHECK(emio::format("{:.3f}", 0.9999) == "1.000");
  CHECK(emio::format("{:.3}", 0.00123) == "0.00123");
  CHECK(emio::format("{:.16g}", 0.1) == "0.1");
  CHECK(emio::format("{:.0}", 1.0) == "1");
  CHECK(emio::format("{:.17f}", 225.51575035152064) == "225.51575035152063720");
  CHECK(emio::format("{:.1f}", -761519619559038.2) == "-761519619559038.2");
  CHECK(emio::format("{}", 1.9156918820264798e-56) == "1.9156918820264798e-56");
  CHECK(emio::format("{:.4f}", 7.2809479766055470e-15) == "0.0000");

  // Trigger a rounding error in Grisu by a specially chosen number.
  CHECK(emio::format("{:f}", 3788512123356.985352) == "3788512123356.985352");
}

TEST_CASE("prettify_float") {
  CHECK(emio::format("{}", 1e-4) == "0.0001");
  CHECK(emio::format("{}", 1e-5) == "1e-05");
  CHECK(emio::format("{}", 1e15) == "1000000000000000");
  CHECK(emio::format("{}", 1e16) == "1e+16");
  CHECK(emio::format("{}", 9.999e-5) == "9.999e-05");
  CHECK(emio::format("{}", 1e10) == "10000000000");
  CHECK(emio::format("{}", 1e11) == "100000000000");
  CHECK(emio::format("{}", 1234e7) == "12340000000");
  CHECK(emio::format("{}", 1234e-2) == "12.34");
  CHECK(emio::format("{}", 1234e-6) == "0.001234");
  //  CHECK(emio::format("{}", 0.1f) == "0.1"); -> float not supported
  CHECK(emio::format("{}", double(0.1f)) == "0.10000000149011612");
  //  CHECK(emio::format("{}", 1.35631564e-19f) == "1.3563156e-19"); -> float not supported
  //  printf("%.15e %.15e\r\n", 1.35631564e-19f, 1.35631564e-19);
}

TEST_CASE("format_nan") {
  double nan = std::numeric_limits<double>::quiet_NaN();
  CHECK(emio::format("{}", nan) == "nan");
  CHECK(emio::format("{:+}", nan) == "+nan");
  CHECK(emio::format("{:+06}", nan) == "  +nan");
  CHECK(emio::format("{:<+06}", nan) == "+nan  ");
  CHECK(emio::format("{:^+06}", nan) == " +nan ");
  CHECK(emio::format("{:>+06}", nan) == "  +nan");
  if (std::signbit(-nan)) {
    CHECK(emio::format("{}", -nan) == "-nan");
    CHECK(emio::format("{:+06}", -nan) == "  -nan");
  } else {
    WARN("compiler doesn't handle negative NaN correctly");
  }
  CHECK(emio::format("{: }", nan) == " nan");
  CHECK(emio::format("{:F}", nan) == "NAN");
  CHECK(emio::format("{:<7}", nan) == "nan    ");
  CHECK(emio::format("{:^7}", nan) == "  nan  ");
  CHECK(emio::format("{:>7}", nan) == "    nan");
  CHECK(emio::format("{:07}", nan) == "    nan");
  CHECK(emio::format("{:x>07}", nan) == "xxxxnan");
}

TEST_CASE("format_infinity") {
  double inf = std::numeric_limits<double>::infinity();
  CHECK(emio::format("{}", inf) == "inf");
  CHECK(emio::format("{:+}", inf) == "+inf");
  CHECK(emio::format("{}", -inf) == "-inf");
  CHECK(emio::format("{:+06}", inf) == "  +inf");
  CHECK(emio::format("{:+06}", -inf) == "  -inf");
  CHECK(emio::format("{:<+06}", inf) == "+inf  ");
  CHECK(emio::format("{:^+06}", inf) == " +inf ");
  CHECK(emio::format("{:>+06}", inf) == "  +inf");
  CHECK(emio::format("{: }", inf) == " inf");
  CHECK(emio::format("{:F}", inf) == "INF");
  CHECK(emio::format("{:<7}", inf) == "inf    ");
  CHECK(emio::format("{:^7}", inf) == "  inf  ");
  CHECK(emio::format("{:>7}", inf) == "    inf");
  CHECK(emio::format("{:07}", inf) == "    inf");
  CHECK(emio::format("{:x>07}", inf) == "xxxxinf");
}

TEST_CASE("format_char") {
  //  const char types[] = "cbBdoxX";
  //  check_unknown_types('a', types, "char");
  CHECK(emio::format("{0}", 'a') == "a");
  CHECK(emio::format("{0:c}", 'z') == "z");
  CHECK(emio::format("{0:d}", '0') == "48");
  CHECK(emio::format("{0:x}", '0') == "30");
  //  int n = 'x';
  //  for (const char* type = types + 1; *type; ++type) {
  //    std::string format_str = fmt::format("{{:{}}}", *type);
  //    EXPECT_EQ(fmt::format(runtime(format_str), n),
  //              fmt::format(runtime(format_str), 'x'))
  //        << format_str;
  //  }
  //  CHECK(emio::format("{:02X}", 'x') == fmt::format("{:02X}", n));

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
  // CHECK(ptr(sp.get())), fmt::format("{}", fmt::ptr(sp)) == fmt::format("{}");
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
  CHECK(emio::format("{}", std::string("test")) == "test");
  CHECK(emio::format("{:s}", std::string("test")) == "test");
  CHECK(emio::format("{:?}", std::string("test")) == "\"test\"");
  CHECK(emio::format("{:*^10?}", std::string("test")) == "**\"test\"**");
  CHECK(emio::format("{:?}", std::string("\test")) == "\"\\test\"");

  CHECK(emio::format(emio::runtime("{:x}"), std::string("test")) == emio::err::invalid_format);
}

TEST_CASE("format_string_view") {
  CHECK(emio::format("{}", std::string_view("test")) == "test");
  CHECK(emio::format("{:s}", std::string_view("test")) == "test");
  CHECK(emio::format("{:.0}", std::string_view("test")) == "");
  CHECK(emio::format("{:.2}", std::string_view("test")) == "te");
  CHECK(emio::format("{:.4}", std::string_view("test")) == "test");
  CHECK(emio::format("{:.6}", std::string_view("test")) == "test");
  CHECK(emio::format("{:?}", std::string_view("t\nst")) == "\"t\\nst\"");
  CHECK(emio::format("{}", std::string_view()) == "");
  CHECK(emio::format("{:?}", std::string_view("t\n\r\t\\\'\"st")) == "\"t\\n\\r\\t\\\\\\'\\\"st\"");
  CHECK(emio::format("{:?}", std::string_view("\x05\xab\xEf")) == "\"\\x05\\xab\\xef\"");

  CHECK(!validate_format_string<std::string_view>("{:.3?}"sv));
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
  CHECK(emio::format("{}", std::string_view("test")) == "test");
  CHECK(emio::format("{}", string_viewable()) == "foo");
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
  CHECK(emio::format("{}", explicitly_convertible_to_std_string_view()) == "'foo'");
}
#endif

TEST_CASE("format at compile-time") {
  constexpr bool success = [] {
    emio::static_buffer<17> buf{};

    emio::result<void> res = emio::format_to(buf, "{} {:.2f} {}{}", 42, 42.24, "x,", 'y');
    return res && buf.view() == "42 42.24 x,y";
  }();
  STATIC_CHECK(success);
}

TEST_CASE("validate_format_string") {
  CHECK(validate_format_string(""sv));
  CHECK(validate_format_string("abc"sv));
  CHECK(!validate_format_string("abc{"sv));
  CHECK(!validate_format_string("abc{x"sv));
  CHECK(validate_format_string("abc{{"sv));
  CHECK(!validate_format_string("abc}"sv));
  CHECK(!validate_format_string("abc}x"sv));
  CHECK(validate_format_string("abc}}"sv));
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
  CHECK(validate_format_string<int>("{:<}"sv));

  CHECK(validate_format_string<int>("{:a<10}"sv));
  CHECK(validate_format_string<int>("{:a^10}"sv));
  CHECK(validate_format_string<int>("{:a>10}"sv));
  CHECK(!validate_format_string<int>("{:a=10}"sv));
  CHECK(validate_format_string<int>("{:a<}"sv));
  CHECK(validate_format_string<int>("{:a<010}"sv));

  CHECK(validate_format_string<int>("{:a<-10}"sv));
  CHECK(validate_format_string<int>("{:a<+10}"sv));
  CHECK(validate_format_string<int>("{:a< 10}"sv));
  CHECK(validate_format_string<int>("{:a<-#10}"sv));
  CHECK(!validate_format_string<int>("{:a<--10}"sv));

  CHECK(validate_format_string<int>("{:+}"sv));
  CHECK(validate_format_string<int>("{:-}"sv));
  CHECK(validate_format_string<int>("{: }"sv));
  CHECK(validate_format_string<int>("{:<}"sv));

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
  CHECK(validate_format_string<int>("{:0}"sv));
  CHECK(!validate_format_string<int>("{:--010}"sv));

  CHECK(!validate_format_string<int>("{:{}}"sv));
  CHECK(!validate_format_string<int>("{:s}"sv));
  CHECK(validate_format_string<double>("{:.1100}"sv));
  CHECK(!validate_format_string<double>("{:.1101}"sv));
  CHECK(!validate_format_string<double>("{:.2147483648}"sv));
  CHECK(!validate_format_string<double>("{:2147483648}"sv));
  CHECK(!validate_format_string<double>("{:.+1}"sv));
  CHECK(!validate_format_string<double>("{:.-1}"sv));

  CHECK(validate_format_string<bool>("{}"sv));
  CHECK(validate_format_string<bool>("{:d}"sv));
  CHECK(!validate_format_string<bool>("{:f}"sv));
  CHECK(!validate_format_string<bool>("{:.5}"sv));

  CHECK(validate_format_string<char>("{}"sv));
  CHECK(validate_format_string<char>("{:d}"sv));

  CHECK(validate_format_string<int>("{}"sv));
  CHECK(validate_format_string<unsigned>("{}"sv));
}
