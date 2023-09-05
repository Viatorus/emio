#include <array>
#include <cinttypes>
#include <cstdio>

int main(int c, char* args[]) {
  std::array<char, 42> s{};
  void* null = nullptr;
  int n = snprintf(s.data(), s.size(), "%d %" PRIu64 " %x %.*s, %p %f %e %g %a", -42, uint64_t{1}, 48, 0, args[0], null,
                   static_cast<double>(c) / 3.14, static_cast<double>(c) / 3.14, static_cast<double>(c) / 3.14,
                   static_cast<double>(c) / 3.14);

  char ch{};
  int8_t i8{};
  uint8_t u8{};
  int16_t i16{};
  uint16_t u16{};
  int32_t i32{};
  uint32_t u32{};
  int64_t i64{};
  uint64_t u64{};
  n += sscanf(args[0], "%c%" SCNd8 "%" SCNu8 "%" SCNd16 "%" SCNu16 "%" SCNd32 "%" SCNu32 "%" SCNd64 "%" SCNu64, &ch,
              &i8, &u8, &i16, &u16, &i32, &u32, &i64, &u64);
  return n;
}
