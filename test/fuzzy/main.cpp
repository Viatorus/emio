#pragma GCC optimize("O3")
#include <climits>

#include "emio/detail/format/decode.hpp"
#include "emio/detail/format/dragon.hpp"

extern "C" {
#include <unistd.h>
}

#ifndef __AFL_INIT
#  define __AFL_INIT() \
    int _i_ = 0;       \
    read(0, &x, 8);
#  define __AFL_FUZZ_INIT() double x{};
#  define __AFL_FUZZ_TESTCASE_BUF &x
#  define __AFL_LOOP(...) _i_++ == 0
#  define __AFL_FUZZ_TESTCASE_LEN 8
#endif

extern "C" {

struct Buffer {
  int16_t k;
  size_t len;
  uint8_t* data;
};

extern Buffer rust_shortest(double d);
extern Buffer rust_fixed(double d, int16_t precision);
extern Buffer rust_exponent(double d, int16_t precision);
extern void rust_free(Buffer b);
}

namespace {

void print_shortest_header(double d) {
  printf("shortest of %.17e\r\n", d);
}

void print_fixed_header(double d, int16_t precision) {
  printf("fixed of %.17e with a precision of %d\r\n", d, precision);
}

void print_exact_header(double d, int16_t precision) {
  printf("exact of %.17e with a length of %d\r\n", d, precision);
}

template <typename T>
void print_result(std::string_view name, std::span<T> digits, int16_t k) {
  printf("%.*s -> digits: %.*s k: %d \r\n", static_cast<int>(name.size()), name.data(), static_cast<int>(digits.size()),
         digits.data(), k);
}

template <typename T>
std::span<T> remove_trailing_zeros(std::span<T> digits) {
  auto it = std::find_if(digits.rbegin(), digits.rend(), [](char c) {
    return c != '0';
  });
  return digits.subspan(0, it.base() - digits.begin());
}

void test_shortest(double d) {
  Buffer b = rust_shortest(d);
  int16_t rust_k = b.k;
  std::span<const char> rust_digits{reinterpret_cast<const char*>(b.data), b.len};

  auto full_decoded = emio::detail::format::decode(d);
  if (full_decoded.category != emio::detail::format::category::finite) {
    if (!rust_digits.empty() || rust_k != 0) {
      print_shortest_header(d);
      print_result("rust", rust_digits, rust_k);
      print_result("emio (not finite)", std::span<char>{}, 0);
      abort();
    }
  } else {
    emio::string_buffer buf;
    auto [digits, k] = emio::detail::format::format_shortest(full_decoded.finite, buf);
    if (!std::equal(rust_digits.begin(), rust_digits.end(), digits.begin(), digits.end()) || rust_k != k) {
      print_shortest_header(d);
      print_result("rust", rust_digits, rust_k);
      print_result("emio", digits, k);
      abort();
    }
  }
  rust_free(b);
}

void test_fixed(double d, int16_t precision) {
  precision = std::clamp<int16_t>(precision, -1000, 1000);

  Buffer b = rust_fixed(d, precision);
  int16_t rust_k = b.k;
  std::span<const char> rust_digits{reinterpret_cast<const char*>(b.data), b.len};
  rust_digits = remove_trailing_zeros(rust_digits);

  auto full_decoded = emio::detail::format::decode(d);
  if (full_decoded.category != emio::detail::format::category::finite) {
    if (!rust_digits.empty() || rust_k != 0) {
      print_fixed_header(d, precision);
      print_result("rust", rust_digits, rust_k);
      print_result("emio (not finite)", std::span<char>{}, 0);
      abort();
    }
  } else {
    emio::string_buffer buf;
    auto [digits, k] = emio::detail::format::format_exact(
        full_decoded.finite, buf, emio::detail::format::format_exact_mode::decimal_point, precision);
    digits = remove_trailing_zeros(digits);

    if (!std::equal(rust_digits.begin(), rust_digits.end(), digits.begin(), digits.end()) || rust_k != k) {
      print_fixed_header(d, precision);
      print_result("rust", rust_digits, rust_k);
      print_result("emio", digits, k);
      abort();
    }
  }
  rust_free(b);
}

void test_exact(double d, int16_t precision) {
  precision = std::clamp<int16_t>(precision, 1, 1000);

  Buffer b = rust_exponent(d, precision);
  int16_t rust_k = b.k;
  std::span<const char> rust_digits{reinterpret_cast<const char*>(b.data), b.len};
  rust_digits = remove_trailing_zeros(rust_digits);

  auto full_decoded = emio::detail::format::decode(d);
  if (full_decoded.category != emio::detail::format::category::finite) {
    if (!rust_digits.empty() || rust_k != 0) {
      print_exact_header(d, precision);
      print_result("rust", rust_digits, rust_k);
      print_result("emio (not finite)", std::span<char>{}, 0);
      abort();
    }
  } else {
    emio::string_buffer buf;
    auto [digits, k] = emio::detail::format::format_exact(
        full_decoded.finite, buf, emio::detail::format::format_exact_mode::significant_digits, precision);
    digits = remove_trailing_zeros(digits);

    if (!std::equal(rust_digits.begin(), rust_digits.end(), digits.begin(), digits.end()) || rust_k != k) {
      print_exact_header(d, precision);
      print_result("rust", rust_digits, rust_k);
      print_result("emio", digits, k);
      abort();
    }
  }
  rust_free(b);
}

}  // namespace

__AFL_FUZZ_INIT();

int main() {
  __AFL_INIT();
  double* magic = (double*)__AFL_FUZZ_TESTCASE_BUF;

  while (__AFL_LOOP(INT_MAX)) {
    int len = __AFL_FUZZ_TESTCASE_LEN;
    if (len < (sizeof(double))) continue;

    double d = *magic;
    test_shortest(d);

    int16_t limit = 17;
    if (len >= (sizeof(double) + sizeof(int16_t))) {
      limit = *((int16_t*)(__AFL_FUZZ_TESTCASE_BUF + sizeof(double)));
    }
    test_fixed(d, limit);
    test_exact(d, limit);
  }

  return 0;
}

// afl-c++ main.cpp -I../../include -std=c++20
// afl-fuzz -i seeds_dir/ -o output_dir/  -- ./a.out
