#include <emio/scan.hpp>

int main(int /*c*/, char* args[]) {
  char ch{};
  int8_t i8{};
  uint8_t u8{};
  int16_t i16{};
  uint16_t u16{};
  int32_t i32{};
  uint32_t u32{};
  int64_t i64{};
  uint64_t u64{};
  static_cast<void>(emio::scan(args[0], "{}{}{}{}{}{}{}{}{}", ch, i8, u8, i16, u16, i32, u32, i64, u64));
}
