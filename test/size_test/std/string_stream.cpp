#include <sstream>

int main() {
  std::stringstream ss;
  ss << 1;
  static_cast<void>(ss.str());
}
