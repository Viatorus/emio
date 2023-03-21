// Unit under test.
#include <emio/detail/bignum.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

TEST_CASE("bignum") {
  using emio::detail::bignum;
  using emio::detail::borrowing_sub;
  using emio::detail::borrowing_sub_result_t;
  using emio::detail::carrying_add;
  using emio::detail::carrying_add_result_t;
  using emio::detail::carrying_mul;
  using emio::detail::carrying_mul_result_t;

  const uint32_t max = std::numeric_limits<uint32_t>::max();

  SECTION("carrying_add") {
    using res_t = carrying_add_result_t;
    CHECK(carrying_add(0, 0, false) == res_t{0, false});
    CHECK(carrying_add(0, 0, true) == res_t{1, false});
    CHECK(carrying_add(1, 0, false) == res_t{1, false});
    CHECK(carrying_add(1, 1, false) == res_t{2, false});
    CHECK(carrying_add(1, 1, true) == res_t{3, false});
    CHECK(carrying_add(max, 1, false) == res_t{0, true});
    CHECK(carrying_add(max, 1, true) == res_t{1, true});
    CHECK(carrying_add(1, max, false) == res_t{0, true});
    CHECK(carrying_add(1, max, true) == res_t{1, true});
    CHECK(carrying_add(max, max, false) == res_t{max - 1, true});
    CHECK(carrying_add(max, max, true) == res_t{max, true});
  }

  SECTION("borrowing_sub") {
    using res_t = borrowing_sub_result_t;
    CHECK(borrowing_sub(0, 0, false) == res_t{0, false});
    CHECK(borrowing_sub(0, 0, true) == res_t{max, true});
    CHECK(borrowing_sub(1, 0, false) == res_t{1, false});
    CHECK(borrowing_sub(1, 1, false) == res_t{0, false});
    CHECK(borrowing_sub(1, 1, true) == res_t{max, true});
    CHECK(borrowing_sub(max, 1, false) == res_t{max - 1, false});
    CHECK(borrowing_sub(max, 1, true) == res_t{max - 2, false});
    CHECK(borrowing_sub(1, max, false) == res_t{2, true});
    CHECK(borrowing_sub(1, max, true) == res_t{1, true});
    CHECK(borrowing_sub(max, max, false) == res_t{0, false});
    CHECK(borrowing_sub(max, max, true) == res_t{max, true});
  }

  SECTION("carrying_mul") {
    using res_t = carrying_mul_result_t;
    CHECK(carrying_mul(0, 0, 0) == res_t{0, 0});
    CHECK(carrying_mul(0, 0, 4) == res_t{4, 0});
    CHECK(carrying_mul(2, 0, 0) == res_t{0, 0});
    CHECK(carrying_mul(2, 3, 0) == res_t{6, 0});
    CHECK(carrying_mul(2, 3, 4) == res_t{10, 0});

    CHECK(carrying_mul(max, max, 0) == res_t{1, max - 1});
    CHECK(carrying_mul(max, max - 1, 0) == res_t{2, max - 2});
    CHECK(carrying_mul(max, max, 9) == res_t{10, max - 1});
    CHECK(carrying_mul(max, max, max) == res_t{0, max});
  }

  SECTION("from small") {
    const uint32_t v = 15;
    const bignum b{v};
    CHECK(b == bignum::from(1, {15, 0}));
  }

  SECTION("from_u64") {
    SECTION("<= 32") {
      const uint64_t v = 15;
      const bignum b{v};
      CHECK(b == bignum::from(1, {15, 0}));
    }
    SECTION(">= 32") {
      const uint64_t v = 592359492949;
      const bignum b{v};
      CHECK(b == bignum::from(2, {3948973397, 137}));
    }
  }

  SECTION("get_bit") {
    const bignum b{0x8004000044000008};
    const std::array bits_set = {63, 50, 30, 26, 3};
    for (size_t i = 0; i < 64; i++) {
      if (std::find(bits_set.begin(), bits_set.end(), i) != bits_set.end()) {
        CHECK(b.get_bit(i) == 1);
      } else {
        CHECK(b.get_bit(i) == 0);
      }
    }
  }

  SECTION("is_zero") {
    CHECK(bignum{0U}.is_zero());
    CHECK(!bignum{129U}.is_zero());
    CHECK(!bignum{592359492949U}.is_zero());
  }

  SECTION("add_small") {
    bignum b{max - 1};
    SECTION("small steps") {
      b.add_small(1);
      CHECK(b == bignum::from(1, {max, 0}));

      b.add_small(3);
      CHECK(b == bignum::from(2, {2, 1}));
    }
    SECTION("one step") {
      b.add_small(4);
      CHECK(b == bignum::from(2, {2, 1}));
    }
  }

  SECTION("add") {
    bignum a{0x1598468598486213U};
    bignum b{0x400200410040010U};
    const bignum c{0x954410040010U};
    const bignum d{0x8918918U};

    SECTION("a + b") {
      CHECK(a.add(b) == bignum{0x19986689a84c6223U});
    }
    SECTION("b + c") {
      CHECK(b.add(c) == bignum{0x400b54820080020U});
    }
    SECTION("a + d") {
      CHECK(a.add(d) == bignum{0x15984685a0d9eb2bU});
    }
  }

  SECTION("sub_small") {
    bignum b = bignum::from(2, {2, 1});
    SECTION("small steps") {
      b.sub_small(1);
      CHECK(b == bignum::from(2, {1, 1}));

      b.sub_small(1);
      CHECK(b == bignum::from(2, {0, 1}));

      b.sub_small(3);
      CHECK(b == bignum::from(1, {max - 2, 0}));
    }
    SECTION("one step") {
      b.sub_small(5);
      CHECK(b == bignum::from(1, {max - 2, 0}));

      b.sub_small(max - 2);
      CHECK(b == bignum::from(1, {0, 0}));
    }
  }

  SECTION("sub") {
    bignum a = bignum::from(3, {4, 1, 2});
    const bignum b = bignum::from(3, {2, 1, 2});
    const bignum c = bignum::from(2, {4, 1});
    const bignum d = bignum::from(1, {2});
    SECTION("a") {
      SECTION("- a") {
        CHECK(a.sub(a) == bignum::from(0, {}));
      }
      SECTION("- b") {
        CHECK(a.sub(b) == bignum::from(1, {2}));
        SECTION("- d") {
          CHECK(a.sub(d) == bignum::from(0, {0}));
        }
      }
      SECTION("- c") {
        CHECK(a.sub(c) == bignum::from(3, {0, 0, 2}));
        SECTION("- c") {
          CHECK(a.sub(c) == bignum::from(3, {max - 3, max - 1, 1}));
          SECTION("- d") {
            CHECK(a.sub(d) == bignum::from(3, {max - 5, max - 1, 1}));
          }
        }
        SECTION("- d") {
          CHECK(a.sub(d) == bignum::from(3, {max - 1, max, 1}));
        }
      }
    }
  }

  SECTION("mul_small") {
    bignum a = bignum::from(3, {4, 1, 2});
    CHECK(a.mul_small(2) == bignum::from(3, {8, 2, 4}));
    CHECK(a.mul_small(3) == bignum::from(3, {24, 6, 12}));
    CHECK(a.mul_small(360000000) == bignum::from(4, {50065408, 2160000002, 25032704, 1}));
    CHECK(a.mul_small(0) == bignum::from(4, {0, 0, 0, 0}));
  }

  SECTION("mul_small_add") {
    bignum a = bignum::from(3, {4, 1, 5});
    CHECK(a.muladd_small(1860000000, 15) == bignum::from(4, {3145032719, 1860000001, 710065408, 2}));
  }

  SECTION("mul") {
    SECTION("test 1") {
      const bignum a = bignum::from(2, {117327783, 607});
      const bignum b = bignum::from(2, {152131585, 547});
      CHECK(a.mul(b) == bignum::from(3, {280407975, 1907502595, 332065}));
    }
    SECTION("test 2") {
      const bignum a = bignum::from(3, {959858, 1547, 499669885});
      const bignum b = bignum::from(6, {18959849, 65907, 594953, 9985999, 5499, 598879596});
      CHECK(a.mul(b) == bignum::from(9, {986308290, 2397938630, 2743294157, 1925473916, 3187470779, 4197912536,
                                         1955690142, 2089129235, 69672730}));
    }
  }

  SECTION("pow5mul") {
    SECTION("test 1") {
      bignum a = bignum::from(1, {2});
      CHECK(a.pow5mul(5) == bignum::from(1, {6250}));
    }
    SECTION("test 2") {
      bignum a = bignum::from(2, {1898, 125});
      CHECK(a.pow5mul(13) == bignum::from(3, {1907158706, 2264035804, 35}));
    }
    SECTION("test 3") {
      bignum a = bignum::from(2, {79581, 498859848});
      CHECK(a.pow5mul(57) ==
            bignum::from(7, {2630842737, 672537777, 2761821040, 1492594830, 2410358682, 1582604142, 2}));
    }
  }

  SECTION("div_rem_small") {
    SECTION("test 1") {
      bignum a{0x1598468598486213U};
      CHECK(a.div_rem_small(0x44F2E41D) == 0xA83F4B9);
      CHECK(a == bignum::from(2, {0x502DEFA2, 0}));
    }
    SECTION("test 2") {
      bignum a{0x400200410040010U};
      CHECK(a.div_rem_small(0x56489623) == 0x450C3720);
      CHECK(a == bignum::from(2, {0xBDE8A50, 0}));
    }
    SECTION("test 3") {
      bignum a{0x1895948989562198U};
      CHECK(a.div_rem_small(0x7) == 2);
      CHECK(a == bignum::from(2, {0x3830E03A, 0x3831538}));
    }
    SECTION("test 4") {
      bignum a{0x1895948989562198U};
      CHECK(a.div_rem_small(0x1) == 0);
      CHECK(a == bignum{0x1895948989562198U});
    }
    SECTION("test 5") {
      bignum a{0x240010U};
      CHECK(a.div_rem_small(0x1548) == 0x148);
      CHECK(a == bignum::from(1, {0x1B1}));
    }
    SECTION("test 6") {
      bignum a{0x240010U};
      CHECK(a.div_rem_small(0x240020) == 0x240010);
      CHECK(a == bignum::from(1, {0}));
    }
  }

  SECTION("mul_pow2") {
    SECTION("test 1") {
      bignum a = bignum::from(3, {0x1, 0x3, 0x2});
      CHECK(a.mul_pow2(1) == bignum::from(3, {0x2, 0x6, 0x4}));
      CHECK(a.mul_pow2(9) == bignum::from(3, {0x400, 0xC00, 0x800}));
      CHECK(a.mul_pow2(32) == bignum::from(4, {0x0, 0x400, 0xC00, 0x800}));
      CHECK(a.mul_pow2(67) == bignum::from(6, {0, 0, 0, 0x2000, 0x6000, 0x4000}));
      CHECK(a.mul_pow2(19) == bignum::from(7, {0, 0, 0, 0, 0x1, 0x3, 0x2}));
    }
  }

  SECTION("three-way comparison") {}
}
