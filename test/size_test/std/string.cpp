#include <string>

int main() {
  std::string s{"abcdefghijklmnopqrstuvwxyz"};
  static_cast<void>(s.c_str());
}
