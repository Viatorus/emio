#include <array>
#include <cinttypes>
#include <cstdio>

int main(int c, char* args[]) {
  std::array<char, 42> s{};
  void* null = nullptr;
  int n = snprintf(s.data(), s.size(), "%d %" PRIu64 " %x %.*s, %p %f %e %g %a", -42, uint64_t{1}, 48, 0, args[0], null,
                   static_cast<double>(c) / 3.14, static_cast<double>(c) / 3.14, static_cast<double>(c) / 3.14,
                   static_cast<double>(c) / 3.14);
  return s[1] + n;
}
