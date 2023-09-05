#include <array>
#include <cinttypes>
#include <cstdio>

int main(int /*c*/, char* args[]) {
  char c{};
  int8_t i8{};
  uint8_t u8{};
  int16_t i16{};
  uint16_t u16{};
  int32_t i32{};
  uint32_t u32{};
  int64_t i64{};
  uint64_t u64{};
  int n = sscanf(args[0], "%c%" SCNd8 "%" SCNu8 "%" SCNd16 "%" SCNu16 "%" SCNd32 "%" SCNu32 "%" SCNd64 "%" SCNu64, &c,
                 &i8, &u8, &i16, &u16, &i32, &u32, &i64, &u64);
  return n;
}
