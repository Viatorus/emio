#include "fmt/format.h"

int main(int c, char*[] /*unused*/) {
  int exp{};
  fmt::basic_memory_buffer b;
  fmt::detail::gen_digits_handler gdh{b.data(), 0, 1, 2, {}};
  fmt::detail::grisu_gen_digits(static_cast<double>(c) / 3.8f, 1, exp, gdh);
  fmt::detail::format_dragon(static_cast<double>(c) / 3.8f, fmt::detail::dragon::fixed, 6, b, exp);
}
