#include <emio/emio.hpp>

int main(int /*c*/, char* args[]) {
  static_cast<void>(emio::format("{} {} {} {} {} {} {} {} {} {} {}", true, static_cast<int8_t>(1),
                                 static_cast<uint8_t>(2), static_cast<int16_t>(3), static_cast<uint16_t>(4),
                                 static_cast<int32_t>(5), static_cast<uint32_t>(6), "abc", 'x', nullptr, 4.2));

  char c{};
  int8_t i8{};
  uint8_t u8{};
  int16_t i16{};
  uint16_t u16{};
  int32_t i32{};
  uint32_t u32{};
  int64_t i64{};
  uint64_t u64{};
  static_cast<void>(emio::scan(args[0], "{}{}{}{}{}{}{}{}{}", c, i8, u8, i16, u16, i32, u32, i64, u64));
}
