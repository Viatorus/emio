#pragma once

#include <cstdint>
#include <tuple>

namespace emio::test {

inline constexpr std::tuple integer_ranges{
    std::tuple{
        std::type_identity<bool>{},
        std::tuple{"1", "0", "-1", "-10"},
        std::tuple{"0", "1", "2", "10"},
    },
    std::tuple{
        std::type_identity<int8_t>{},
        std::tuple{"-127", "-128", "-129", "-1280"},
        std::tuple{"126", "127", "128", "1270"},
    },
    std::tuple{
        std::type_identity<uint8_t>{},
        std::tuple{"1", "0", "-1", "-10"},
        std::tuple{"254", "255", "256", "2550"},
    },
    std::tuple{
        std::type_identity<int16_t>{},
        std::tuple{"-32767", "-32768", "-32769", "-327680"},
        std::tuple{"32766", "32767", "327678", "327670"},
    },
    std::tuple{
        std::type_identity<uint16_t>{},
        std::tuple{"1", "0", "-1", "-10"},
        std::tuple{"65534", "65535", "65536", "655350"},
    },
    std::tuple{
        std::type_identity<int32_t>{},
        std::tuple{"-2147483647", "-2147483648", "-2147483649", "-21474836480"},
        std::tuple{"2147483646", "2147483647", "2147483648", "21474836470"},
    },
    std::tuple{
        std::type_identity<uint32_t>{},
        std::tuple{"1", "0", "-1", "-10"},
        std::tuple{"4294967294", "4294967295", "4294967296", "42949672950"},
    },
    std::tuple{
        std::type_identity<int64_t>{},
        std::tuple{"-9223372036854775807", "-9223372036854775808", "-9223372036854775809", "-92233720368547758080"},
        std::tuple{"9223372036854775806", "9223372036854775807", "9223372036854775808", "-92233720368547758070"},
    },
    std::tuple{
        std::type_identity<uint64_t>{},
        std::tuple{"1", "0", "-1", "-10"},
        std::tuple{"18446744073709551614", "18446744073709551615", "18446744073709551616", "184467440737095516150"},
    },
};

template <typename T>
constexpr void apply_integer_ranges(T&& func) {
  std::apply(
      [&](auto... inputs) {
        (std::apply(func, inputs), ...);
      },
      integer_ranges);
}

}  // namespace emio::test
