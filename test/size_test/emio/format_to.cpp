#include <array>
#include <emio/format.hpp>

int main() {
  std::array<char, 1> arr;
  emio::span_buffer buf{arr};
  emio::format_to(buf, "{}", 1).value();
}
