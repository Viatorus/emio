#include <fmt/core.h>

int main() {
  static_cast<void>(fmt::format(fmt::runtime("{}"), 1));
}
