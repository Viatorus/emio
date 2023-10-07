#include <climits>

#include "emio/emio.hpp"

extern "C" {
#include <unistd.h>
}

#ifndef __AFL_INIT
#  define __AFL_INIT() \
    int _i_ = 0;       \
    read(0, &x, 8);
#  define __AFL_FUZZ_INIT() double x{};
#  define __AFL_FUZZ_TESTCASE_BUF &x
#  define __AFL_LOOP(...) _i_++ == 0
#  define __AFL_FUZZ_TESTCASE_LEN 8
#endif

namespace {

constexpr std::tuple<bool, int32_t, int64_t, uint32_t, uint64_t, char, std::string_view, double, double> combinations =
    {true, 4589, 4986498846, 176598, 985486, 'y', "gneqo nvqoqno", 1.59879, std::numeric_limits<double>::infinity()};

template <typename... Args>
void format(emio::runtime_string format_string, Args&&... args) {
  static_cast<void>(emio::format(format_string, args...));
  if constexpr (sizeof...(Args) < 3) {  // Unpack tuple 3 times.
    std::apply(
        [&](auto... arg) {
          (..., format(format_string, args..., arg));
        },
        combinations);
  }
}

template <typename Arg>
void random_format(emio::runtime_string format_string, int64_t storage, Arg arg) {
  if constexpr (std::is_same_v<Arg, std::string_view>) {
    arg = format_string.view();
  } else {
    memcpy(&arg, &storage, sizeof(Arg));
  }
  format(format_string, arg);
}

}  // namespace

__AFL_FUZZ_INIT();

int main() {
  __AFL_INIT();
  const char* buf = (const char*)__AFL_FUZZ_TESTCASE_BUF;

  while (__AFL_LOOP(INT_MAX)) {
    size_t len = __AFL_FUZZ_TESTCASE_LEN;

    std::string_view str{buf, len};
    auto format_string = emio::runtime(str);
    static_cast<void>(emio::format(format_string));

    // Random arg.
    int64_t arg_storage{};
    memcpy(&arg_storage, buf, std::min(sizeof(int64_t), len));

    std::apply(
        [&](auto... arg) {
          (..., random_format(format_string, arg_storage, arg));
        },
        combinations);
  }

  return 0;
}
