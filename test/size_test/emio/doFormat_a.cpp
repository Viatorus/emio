#include <emio/format.hpp>

int main() {
  emio::format("{}\n", "somefile.cpp").value();
  emio::format("{}:{}\n", "somefile.cpp", 42).value();
  emio::format("{}:{}:{}\n", "somefile.cpp", 42, "asdf").value();
  emio::format("{}:{}:{}:{}\n", "somefile.cpp", 42, 1, "asdf").value();
  emio::format("{}:{}:{}:{}:{}\n", "somefile.cpp", 42, 1, 2, "asdf").value();
}
