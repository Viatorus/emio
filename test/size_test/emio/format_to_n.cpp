#include <array>
#include <emio/format.hpp>

int main() {
  std::array<char, 1> arr;
  emio::format_to_n(arr.begin(), 1, "{}", 1).value();
}
