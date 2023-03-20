#include <string>

int main(int c, char*[] /*unused*/) {
  std::string s;
  s.resize(42);
  sprintf(s.data(), "%g", static_cast<double>(c) / 3.14);
  return s[1];
}
