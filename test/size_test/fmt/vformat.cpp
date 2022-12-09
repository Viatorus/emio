#include <fmt/core.h>

int main() {
  static_cast<void>(fmt::vformat("{}", fmt::make_format_args(1)));
}
