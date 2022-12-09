#include <locale>

int main() {
  std::locale l;
  static_cast<void>(l.name());
}
