// Include all.
#include <emio/emio.hpp>

consteval emio::result<void> compile_time_test() {
  emio::static_buffer<128> buf;
  EMIO_TRYV(emio::format_to(buf, "abc"));
  return emio::format_to(buf, "abc {}", 13);
}

emio::result<void> test() {
  EMIO_TRYV(compile_time_test());
  if (emio::format("abc").size() != 3) {
    return emio::err::invalid_data;
  }
  if (emio::format("{}", "abc").size() != 3) {
    return emio::err::invalid_data;
  }
  return emio::format(emio::runtime("abc {}"), 13);
}

int main() {
  if (test().has_error()) {
    return 1;
  }
  return 0;
}
