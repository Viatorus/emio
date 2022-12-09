#include <emio/format.hpp>

int main() {
  emio::format("{}", 1).value();
  emio::format("{}", 2).value();
}
