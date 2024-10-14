#include <emio/emio.hpp>

extern "C" {
void _exit(int /*unused*/) {
  while (1) {
  }
}

void* _sbrk(int /*unused*/) {
  return 0;
}
}

int main() {
  emio::static_buffer<32> buffer;
  const auto result = emio::format_to(buffer, "Test: {:d}\n", 42);
  if (!result) {
    return -1;
  }
  return 0;
}
