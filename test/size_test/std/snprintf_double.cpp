#include <cstdio>
#include <array>

int main(int c, char*[] /*unused*/) {
  std::array<char, 42> s{};
  snprintf(s.data(), s.size(), "%g", static_cast<double>(c) / 3.14);
  return s[1];
}
