#include "fmt/format.h"

int main(int c, char*[] /*unused*/) {
  int exp{};
  fmt::basic_memory_buffer<char> b;
  fmt::detail::format_dragon(static_cast<double>(c) / 3.8f, fmt::detail::dragon::fixed, 6, b, exp);
}
