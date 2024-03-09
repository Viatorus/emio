//
// Created by neubertt on 09.03.2024.
//
#include "emio/emio.hpp"

consteval auto created_with_span_buffer() {
  std::array<char, 5> storage{};
  emio::span_buffer buf{storage};
  emio::result<std::span<char>> area = buf.get_write_area_of(4);
  return storage;
}

int main() {
  created_with_span_buffer();
}
