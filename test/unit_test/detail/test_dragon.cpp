// Unit under test.
#include "emio/detail/format/dragon.hpp"

// Other includes.
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators_all.hpp"

namespace emf = emio::detail::format;

static void check_exact_one(const emf::finite_result_t& decoded, std::string_view expected_str, int16_t expected_k) {
  SECTION("exponent") {
    emio::memory_buffer<char> buf;
    const auto number_of_digits = static_cast<int16_t>(expected_str.size());
    auto [str, k] = emf::format_exact(decoded, buf, emf::format_exact_mode::significand_digits, number_of_digits);
    CHECK(std::string_view(str.begin(), str.end()) == expected_str);
    CHECK(k == expected_k);
  }

  SECTION("fixed") {
    emio::memory_buffer<char> buf;
    const auto number_of_digits = static_cast<int16_t>(-expected_k + static_cast<int16_t>(expected_str.size()));
    auto [str, k] = emf::format_exact(decoded, buf, emf::format_exact_mode::decimal_point, number_of_digits);

    CHECK(std::string_view(str.begin(), str.end()) == expected_str);
    CHECK(k == expected_k);
  }
}

#define CHECK_EXACT(d, exp_str, exp_k)                  \
  SECTION(std::to_string(d)) {                          \
    const auto decoded = emf::decode(d);                \
    REQUIRE(decoded.category == emf::category::finite); \
    check_exact_one(decoded.finite, exp_str, exp_k);    \
  }

#define CHECK_EXACT_ONE(mant_, exp_, exp_str, exp_k)                  \
  SECTION(std::to_string(mant_) + " * 2 ^ " + std::to_string(exp_)) { \
    const auto decoded = emf::decode(std::ldexp(mant_, exp_));        \
    REQUIRE(decoded.category == emf::category::finite);               \
    check_exact_one(decoded.finite, exp_str, exp_k);                  \
  }

TEST_CASE("format_exact") {
  CHECK_EXACT(0.1, "1000000000000000055511151231257827021182", 0);
  CHECK_EXACT(0.45, "4500000000000000111022302462515654042363", 0);
  CHECK_EXACT(0.5, "5000000000000000000000000000000000000000", 0);
  CHECK_EXACT(0.95, "9499999999999999555910790149937383830547", 0);
  CHECK_EXACT(100.0, "1000000000000000000000000000000000000000", 3);
  CHECK_EXACT(999.5, "9995000000000000000000000000000000000000", 3);
  CHECK_EXACT(1.0 / 3.0, "3333333333333333148296162562473909929395", 0);
  CHECK_EXACT(3.141592, "3141592000000000162174274009885266423225", 1);
  CHECK_EXACT(3.141592e17, "3141592000000000000000000000000000000000", 18);
  CHECK_EXACT(1.0e23, "9999999999999999161139200000000000000000", 23);
  CHECK_EXACT(std::numeric_limits<double>::max(), "1797693134862315708145274237317043567981", 309);
  CHECK_EXACT(std::numeric_limits<double>::min(), "2225073858507201383090232717332404064219", -307);
  CHECK_EXACT(std::ldexp(1.0, -1074),
              "4940656458412465441765687928682213723650"
              "5980261432476442558568250067550727020875"
              "1865299836361635992379796564695445717730"
              "9266567103559397963987747960107818781263"
              "0071319031140452784581716784898210368871"
              "8636056998730723050006387409153564984387"
              "3124733972731696151400317153853980741262"
              "3856559117102665855668676818703956031062"
              "4931945271591492455329305456544401127480"
              "1297099995419319894090804165633245247571"
              "4786901472678015935523861155013480352649"
              "3472019379026810710749170333222684475333"
              "5720832431936092382893458368060106011506"
              "1698097530783422773183292479049825247307"
              "7637592724787465608477820373446969953364"
              "7017972677717585125660551199131504891101"
              "4510378627381672509558373897335989936648"
              "0994116420570263709027924276754456522908"
              "7538682506419718265533447265625000000000",
              -323);

  // [1], Table 3: Stress Inputs for Converting 53-bit Binary to Decimal, < 1/2 ULP
  CHECK_EXACT_ONE(8511030020275656, -342, "9", -87);
  CHECK_EXACT_ONE(5201988407066741, -824, "46", -232);
  CHECK_EXACT_ONE(6406892948269899, 237, "141", 88);
  CHECK_EXACT_ONE(8431154198732492, 72, "3981", 38);
  CHECK_EXACT_ONE(6475049196144587, 99, "41040", 46);
  CHECK_EXACT_ONE(8274307542972842, 726, "292084", 235);
  CHECK_EXACT_ONE(5381065484265332, -456, "2891946", -121);
  CHECK_EXACT_ONE(6761728585499734, -1057, "43787718", -302);
  CHECK_EXACT_ONE(7976538478610756, 376, "122770163", 130);
  CHECK_EXACT_ONE(5982403858958067, 377, "1841552452", 130);
  CHECK_EXACT_ONE(5536995190630837, 93, "54835744350", 44);
  CHECK_EXACT_ONE(7225450889282194, 710, "389190181146", 230);
  CHECK_EXACT_ONE(7225450889282194, 709, "1945950905732", 230);
  CHECK_EXACT_ONE(8703372741147379, 117, "14460958381605", 52);
  CHECK_EXACT_ONE(8944262675275217, -1001, "417367747458531", -285);
  CHECK_EXACT_ONE(7459803696087692, -707, "1107950772878888", -196);
  CHECK_EXACT_ONE(6080469016670379, -381, "12345501366327440", -98);
  CHECK_EXACT_ONE(8385515147034757, 721, "925031711960365024", 233);
  CHECK_EXACT_ONE(7514216811389786, -828, "4198047150284889840", -233);
  CHECK_EXACT_ONE(8397297803260511, -345, "11716315319786511046", -87);
  CHECK_EXACT_ONE(6733459239310543, 202, "432810072844612493629", 77);
  CHECK_EXACT_ONE(8091450587292794, -473, "3317710118160031081518", -126);

  // [1], Table 4: Stress Inputs for Converting 53-bit Binary to Decimal, > 1/2 ULP
  CHECK_EXACT_ONE(6567258882077402, 952, "3", 303);
  CHECK_EXACT_ONE(6712731423444934, 535, "76", 177);
  CHECK_EXACT_ONE(6712731423444934, 534, "378", 177);
  CHECK_EXACT_ONE(5298405411573037, -957, "4350", -272);
  CHECK_EXACT_ONE(5137311167659507, -144, "23037", -27);
  CHECK_EXACT_ONE(6722280709661868, 363, "126301", 126);
  CHECK_EXACT_ONE(5344436398034927, -169, "7142211", -35);
  CHECK_EXACT_ONE(8369123604277281, -853, "13934574", -240);
  CHECK_EXACT_ONE(8995822108487663, -780, "141463449", -218);
  CHECK_EXACT_ONE(8942832835564782, -383, "4539277920", -99);
  CHECK_EXACT_ONE(8942832835564782, -384, "22696389598", -99);
  CHECK_EXACT_ONE(8942832835564782, -385, "113481947988", -99);
  CHECK_EXACT_ONE(6965949469487146, -249, "7700366561890", -59);
  CHECK_EXACT_ONE(6965949469487146, -250, "38501832809448", -59);
  CHECK_EXACT_ONE(6965949469487146, -251, "192509164047238", -59);
  CHECK_EXACT_ONE(7487252720986826, 548, "6898586531774201", 181);
  CHECK_EXACT_ONE(5592117679628511, 164, "13076622631878654", 66);
  CHECK_EXACT_ONE(8887055249355788, 665, "136052020756121240", 217);
  CHECK_EXACT_ONE(6994187472632449, 690, "3592810217475959676", 224);
  CHECK_EXACT_ONE(8797576579012143, 588, "89125197712484551899", 193);
  CHECK_EXACT_ONE(7363326733505337, 272, "558769757362301140950", 98);
  CHECK_EXACT_ONE(8549497411294502, -448, "1176257830728540379990", -118);
}

void check_shortest(double d, std::string_view expected_str, int16_t expected_k) {
  const auto decoded = emf::decode(d);
  REQUIRE(decoded.category == emf::category::finite);

  emio::memory_buffer<char> buf;
  auto [str, k] = emf::format_shortest(decoded.finite, buf);
  CHECK(std::string_view(str.begin(), str.end()) == expected_str);
  CHECK(k == expected_k);
}

#define CHECK_SHORTEST(d, exp_str, exp_k) \
  SECTION(#d) { check_shortest(d, exp_str, exp_k); }

TEST_CASE("format_shortest") {
  // 0.0999999999999999777955395074968691915273...
  // 0.1000000000000000055511151231257827021181...
  // 0.1000000000000000333066907387546962127089...
  CHECK_SHORTEST(0.1, "1", 0);

  // this example is explicitly mentioned in the paper.
  // 10^3 * 0.0999999999999999857891452847979962825775...
  // 10^3 * 0.1 (exact)
  // 10^3 * 0.1000000000000000142108547152020037174224...
  CHECK_SHORTEST(100.0, "1", 3);

  // 0.3333333333333332593184650249895639717578...
  // 0.3333333333333333148296162562473909929394... (1/3 in the default rounding)
  // 0.3333333333333333703407674875052180141210...
  CHECK_SHORTEST(1.0 / 3.0, "3333333333333333", 0);

  // explicit test case for equally closest representations.
  // Dragon has its own tie-breaking rule; Grisu should fall back.
  // 10^1 * 0.1000007629394531027955395074968691915273...
  // 10^1 * 0.100000762939453125 (exact)
  // 10^1 * 0.1000007629394531472044604925031308084726...
  CHECK_SHORTEST(1.00000762939453125, "10000076293945313", 1);

  // 10^1 * 0.3141591999999999718085064159822650253772...
  // 10^1 * 0.3141592000000000162174274009885266423225...
  // 10^1 * 0.3141592000000000606263483859947882592678...
  CHECK_SHORTEST(3.141592, "3141592", 1);

  // 10^18 * 0.314159199999999936
  // 10^18 * 0.3141592 (exact)
  // 10^18 * 0.314159200000000064
  CHECK_SHORTEST(3.141592e17, "3141592", 18);

  // regression test for decoders
  // 10^20 * 0.18446744073709549568
  // 10^20 * 0.18446744073709551616
  // 10^20 * 0.18446744073709555712
  CHECK_SHORTEST(ldexp(1.0, 64), "18446744073709552", 20);

  // pathological case: high = 10^23 (exact). tie breaking should always prefer that.
  // 10^24 * 0.099999999999999974834176
  // 10^24 * 0.099999999999999991611392
  // 10^24 * 0.100000000000000008388608
  CHECK_SHORTEST(1.0e23, "1", 24);

  // 10^309 * 0.1797693134862315508561243283845062402343...
  // 10^309 * 0.1797693134862315708145274237317043567980...
  // 10^309 * 0.1797693134862315907729305190789024733617...
  CHECK_SHORTEST(std::numeric_limits<double>::max(), "17976931348623157", 309);

  // 10^-307 * 0.2225073858507200889024586876085859887650...
  // 10^-307 * 0.2225073858507201383090232717332404064219...
  // 10^-307 * 0.2225073858507201877155878558578948240788...
  CHECK_SHORTEST(std::numeric_limits<double>::min(), "22250738585072014", -307);

  // 10^-323 * 0
  // 10^-323 * 0.4940656458412465441765687928682213723650...
  // 10^-323 * 0.9881312916824930883531375857364427447301...
  CHECK_SHORTEST(ldexp(1.0, -1074), "5", -323);
}

void check_fixed(const emf::finite_result_t& finite, int16_t precision, std::string_view expected_str,
                 int16_t expected_k) {
  emio::memory_buffer<char> buf;
  auto [str, k] = emf::format_exact(finite, buf, emf::format_exact_mode::decimal_point, precision);

  CHECK(std::string_view(str.begin(), str.end()) == expected_str);
  CHECK(k == expected_k);
}

void check_exponent(const emf::finite_result_t& finite, int16_t precision, std::string_view expected_str,
                    int16_t expected_k) {
  emio::memory_buffer<char> buf;
  auto [str, k] = emf::format_exact(finite, buf, emf::format_exact_mode::significand_digits, precision);

  CHECK(std::string_view(str.begin(), str.end()) == expected_str);
  CHECK(k == expected_k);
}

#define CHECK_FIXED(d, prec, exp_str, exp_k)                      \
  SECTION(std::to_string(d) + " prec: " + std::to_string(prec)) { \
    const auto decoded = emf::decode(d);                          \
    REQUIRE(decoded.category == emf::category::finite);           \
    check_fixed(decoded.finite, prec, exp_str, exp_k);            \
  }

#define CHECK_EXPONENT(d, prec, exp_str, exp_k)                   \
  SECTION(std::to_string(d) + " prec: " + std::to_string(prec)) { \
    const auto decoded = emf::decode(d);                          \
    REQUIRE(decoded.category == emf::category::finite);           \
    check_exponent(decoded.finite, prec, exp_str, exp_k);         \
  }

TEST_CASE("format_shortest_additional") {
  CHECK_SHORTEST(5.433374549648463e-309, "5433374549648463", -308);

  CHECK_EXPONENT(7.55997183139191130e-306, 0, "", -304);
  CHECK_EXPONENT(7.55997183139191130e-306, 1, "8", -305);
  CHECK_EXPONENT(7.55997183139191130e-306, 2, "76", -305);
  CHECK_EXPONENT(7.55997183139191130e-306, 3, "756", -305);
  CHECK_EXPONENT(7.55997183139191130e-306, 4, "7560", -305);
  CHECK_EXPONENT(7.55997183139191130e-306, 5, "75600", -305);
  CHECK_EXPONENT(7.55997183139191130e-306, 6, "755997", -305);

  CHECK_EXPONENT(9.99999999999982292e-02, 17, "99999999999998229", -1);

  CHECK_EXPONENT(-4.57218091692071384e+303, 0, "", 304);
  CHECK_EXPONENT(-4.57218091692071384e+303, 1, "5", 304);
  CHECK_EXPONENT(-4.57218091692071384e+303, 2, "46", 304);
  CHECK_EXPONENT(-4.57218091692071384e+303, 3, "457", 304);

  // The following test case results into "" instead of "1" if executed with Rust's dragon implementation.
  // Because zero significand digits is an unusual edge case which doesn't make much sense.
  CHECK_EXPONENT(-5.57218091692071384e+303, 0, "1", 305);

  CHECK_EXPONENT(-5.57218091692071384e+303, 1, "6", 304);
  CHECK_EXPONENT(-5.57218091692071384e+303, 2, "56", 304);
  CHECK_EXPONENT(-5.57218091692071384e+303, 3, "557", 304);

  CHECK_FIXED(2.90004715841907341e-57, 15, "", -56);
  CHECK_FIXED(8.984564273899573482e-19, 18, "1", -17);
  CHECK_FIXED(9.55393266803182487e+04, 16, "955393266803182486910", 5);
  CHECK_FIXED(5.41843844705283309e-17, 16, "1", -15);
  CHECK_FIXED(7.55997183139191130e-306, 16, "", -304);
}
