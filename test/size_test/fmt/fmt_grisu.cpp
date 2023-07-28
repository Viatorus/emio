#include "fmt/format.h"

int main(int c, char*[] /*unused*/) {
  int exp{};
  fmt::basic_memory_buffer<char> b;
  fmt::detail::gen_digits_handler gdh{b.data(), 0, 1, 2, {}};
  fmt::detail::grisu_gen_digits(static_cast<double>(c) / 3.8f, 1, exp, gdh);
}
