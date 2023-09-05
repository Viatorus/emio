#include <emio/scan.hpp>

int main(int /*c*/, char* args[]) {
  int i{};
  static_cast<void>(emio::scan(args[0], "{}", i));
}
