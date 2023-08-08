#include <emio/scan.hpp>

int main() {
  int i{};
  static_cast<void>(emio::scan("1", "{}", i));
}
