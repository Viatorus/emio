#include <charconv>
#include <string>

int main() {
  std::string s;
  s.resize(42);
  std::to_chars(s.data(), s.data() + 10, 1.42);
  return s[1];
}
