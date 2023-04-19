#include <emio/format.hpp>

int main() {
  emio::memory_buffer<char> buf;
  emio::writer wrt{buf};
  wrt.write_int(1).value();
  static_cast<void>(buf.str());
}
