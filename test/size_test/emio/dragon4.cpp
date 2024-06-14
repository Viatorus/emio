#include <emio/format.hpp>

int main() {
  using namespace emio::detail::format;

  const decode_result_t decoded = decode(1.8984);
  emio::static_buffer<128> buf;
  format_shortest(decoded.finite, buf);
  format_exact(decoded.finite, buf, format_exact_mode::decimal_point, 18);
}
