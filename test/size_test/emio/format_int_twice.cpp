#include <emio/format.hpp>

int main() {
  static_cast<void>(emio::format("{}", 1));
  static_cast<void>(emio::format("{}", 2));
}
