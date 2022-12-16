#include <emio/format.hpp>

int main() {
  static_cast<void>(emio::format("{} {} {} {} {} {} {} {} {} {}", true, static_cast<int8_t>(1), static_cast<uint8_t>(2),
                                 static_cast<int16_t>(3), static_cast<uint16_t>(4), static_cast<int32_t>(5),
                                 static_cast<uint32_t>(6), "abc", 'x', nullptr));
}
