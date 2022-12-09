#include <fmt/core.h>

#include <array>

int main() {
  std::array<char, 1> arr;
  static_cast<void>(fmt::format_to_n(arr.begin(), 1, "{}", 1));
}
