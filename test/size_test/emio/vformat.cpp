#include <emio/format.hpp>

int main() {
  emio::vformat(emio::make_format_args("{}", 1)).value();
}
