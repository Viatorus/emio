#include <fmt/core.h>

int main() {
  static_cast<void>(fmt::format("{}", 1));
  static_cast<void>(fmt::format("{}", 2));
}
