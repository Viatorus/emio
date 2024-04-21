#include <emio/emio.hpp>

consteval auto created_with_span_buffer() {
  std::array<char, 5> storage{};
  emio::span_buffer buf{storage};
  return storage;
}

consteval auto created_with_static_buffer() {
  std::array<char, 5> storage{};
  emio::static_buffer<5> buf{};
  std::span<char> area = buf.get_write_area_of(5).value();
  std::fill(area.begin(), area.end(), 'a');
  std::copy(buf.view().begin(), buf.view().end(), storage.begin());
  return storage;
}

consteval auto created_with_memory_buffer() {
  std::array<char, 5> storage{};
  emio::memory_buffer<0> buf{};
  std::span<char> area = buf.get_write_area_of(5).value();
  std::fill(area.begin(), area.end(), 'a');
  std::copy(buf.view().begin(), buf.view().end(), storage.begin());
  return storage;
}

consteval auto created_with_iterator_buffer() {
  std::array<char, 5> storage{};
  emio::iterator_buffer buf{storage.data()};
  std::span<char> area = buf.get_write_area_of(5).value();
  std::fill(area.begin(), area.end(), 'a');
  return storage;
}

extern "C" int main() {
  const auto a = created_with_span_buffer();
  const auto b = created_with_static_buffer();
  const auto c = created_with_memory_buffer();
  const auto d = created_with_iterator_buffer();

  return a.size() + b.size() + c.size() + d.size();
}
