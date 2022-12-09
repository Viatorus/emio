#include <emio/format.hpp>

int main() {
  emio::format(emio::runtime("{}"), 1).value();
}
