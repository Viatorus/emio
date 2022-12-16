#include <emio/format.hpp>

int main() {
  static_cast<void>(emio::format("{}\n", "somefile.cpp"));
  static_cast<void>(emio::format("{}:{}\n", "somefile.cpp", 42));
  static_cast<void>(emio::format("{}:{}:{}\n", "somefile.cpp", 42, "asdf"));
  static_cast<void>(emio::format("{}:{}:{}:{}\n", "somefile.cpp", 42, 1, "asdf"));
  static_cast<void>(emio::format("{}:{}:{}:{}:{}\n", "somefile.cpp", 42, 1, 2, "asdf"));
}
