//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include "detail/basic_string.hpp"
#include "result.hpp"

namespace emio {

/**
 * This class provides the basic API and functionality for receiving a contiguous memory region to write into.
 * @note Use a specific subclass for a concrete instantiation.
 * @tparam Char The character type.
 */
template <typename Char = char>
class buffer {
 public:
  buffer(const buffer& other) = delete;
  buffer(buffer&& other) = delete;
  buffer& operator=(const buffer& other) = delete;
  buffer& operator=(buffer&& other) = delete;
  constexpr virtual ~buffer() = default;

  /**
   * Returns a write area with the requested size on success.
   * @param size The size the write area should have.
   * @return The write area with the requested size on success or eof if no write area is available.
   */
  constexpr result<std::span<Char>> get_write_area_of(size_t size) noexcept {
    EMIO_TRY(const std::span<Char> area, get_write_area_of_max(size));
    if (area.size() < size) {
      used_ -= area.size();
      return err::eof;
    }
    return area;
  }

  /**
   * Returns a write area which may be smaller than the requested size.
   * @note This function should be used to support subclasses with a limited internal buffer.
   * E.g. Writing a long string in chunks.
   * @param size The size the write area should maximal have.
   * @return The write area with the requested size as maximum on success or eof if no write area is available.
   */
  constexpr result<std::span<Char>> get_write_area_of_max(size_t size) noexcept {
    const size_t remaining_capacity = area_.size() - used_;
    if (remaining_capacity >= size) {
      const std::span<char> area = area_.subspan(used_, size);
      used_ += size;
      return area;
    }
    EMIO_TRY(const std::span<Char> area, request_write_area(used_, size));
    used_ += area.size();
    return area;
  }

 protected:
  /**
   * Default constructs the buffer.
   */
  constexpr buffer() = default;

  /**
   * Requests a write area of the given size from a subclass.
   * @param used Already written characters into the current write area.
   * @param size The requested size of a new write area.
   * @return The write area with the requested size as maximum on success or eof if no write area is available.
   */
  constexpr virtual result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept {
    static_cast<void>(used);  // Keep params for documentation.
    static_cast<void>(size);
    return err::eof;
  }

  /**
   * Sets a new write area in the base class object to use.
   * @param area The new write area.
   */
  constexpr void set_write_area(const std::span<Char> area) noexcept {
    area_ = area;
    used_ = 0;
  }

  /**
   * Returns the count of written characters of the current hold write area.
   * @return The count.
   */
  [[nodiscard]] constexpr size_t get_used_count() const noexcept {
    return used_;
  }

 private:
  size_t used_{};
  std::span<Char> area_{};
};

/**
 * This class fulfills the buffer API by internal using a string object.
 * @tparam Char The character type.
 */
template <typename Char = char>
class string_buffer final : public buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with the basic capacity a string.
   */
  constexpr string_buffer() : string_buffer{0} {}

  /**
   * Constructs and initializes the buffer with the given capacity.
   * @param capacity The initial capacity of the string.
   */
  constexpr explicit string_buffer(const size_t capacity) {
    // Request at least the SBO capacity.
    static_cast<void>(request_write_area(0, std::max(data_.capacity(), capacity)));
  }

  constexpr string_buffer(const string_buffer&) = default;
  constexpr string_buffer(string_buffer&&) noexcept = default;
  constexpr string_buffer& operator=(const string_buffer&) = default;
  constexpr string_buffer& operator=(string_buffer&&) noexcept = default;
  constexpr ~string_buffer() override = default;

  /**
   * Obtains a view over the underlying string object.
   * @return The view.
   */
  [[nodiscard]] constexpr std::basic_string_view<Char> view() const noexcept {
    return {data_.data(), used_ + this->get_used_count()};
  }

  /**
   * Obtains a copy of the underlying string object.
   * @return The string.
   */
  [[nodiscard]] constexpr std::basic_string<Char> str() const {
    return std::string{view()};
  }

 protected:
  constexpr result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept override {
    const size_t new_size = data_.size() + size;
    data_.resize(new_size);
    used_ += used;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic): Performance in debug.
    const std::span<Char> area{data_.data() + used_, size};
    this->set_write_area(area);
    return area;
  }

 private:
  size_t used_{};
  detail::basic_string_union<Char> data_{};
};

/**
 * This class fulfills the buffer API by using a span over an contiguous range.
 * @tparam Char The character type.
 */
template <typename Char = char>
class span_buffer final : public buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with an empty span.
   */
  constexpr span_buffer() = default;

  /**
   * Constructs and initializes the buffer with the given span.
   * @param span The span.
   */
  constexpr explicit span_buffer(const std::span<Char> span) : span_{span} {
    this->set_write_area(span_);
  }

  constexpr span_buffer(const span_buffer&) = default;
  constexpr span_buffer(span_buffer&&) noexcept = default;
  constexpr span_buffer& operator=(const span_buffer&) = default;
  constexpr span_buffer& operator=(span_buffer&&) noexcept = default;
  constexpr ~span_buffer() override = default;

  /**
   * Obtains a view over the underlying string object.
   * @return The view.
   */
  [[nodiscard]] constexpr std::string_view view() const noexcept {
    return {span_.data(), this->get_used_count()};
  }

  /**
   * Obtains a copy of the underlying string object.
   * @return The string.
   */
  [[nodiscard]] std::basic_string<Char> str() const {
    return std::string{view()};
  }

 private:
  std::span<Char> span_;
};

// Deduction guides.
template <typename T>
  requires std::is_constructible_v<std::span<char>, T>
span_buffer(T&&) -> span_buffer<char>;

template <typename T>
  requires std::is_constructible_v<std::span<char8_t>, T>
span_buffer(T&&) -> span_buffer<char8_t>;

template <typename T>
  requires std::is_constructible_v<std::span<char16_t>, T>
span_buffer(T&&) -> span_buffer<char16_t>;

template <typename T>
  requires std::is_constructible_v<std::span<char32_t>, T>
span_buffer(T&&) -> span_buffer<char32_t>;

namespace detail {

inline constexpr size_t internal_buffer_size{256};

// Extracts a reference to the container from back_insert_iterator.
template <typename Container>
Container& get_container(std::back_insert_iterator<Container> it) {
  using bi_iterator = std::back_insert_iterator<Container>;
  struct accessor : bi_iterator {
    accessor(bi_iterator iter) : bi_iterator(iter) {}
    using bi_iterator::container;
  };
  return *accessor{it}.container;
}

// Helper struct to get the value type of different iterators.
template <typename T>
struct get_value_type {
  using type = typename std::iterator_traits<T>::value_type;
};

template <typename Container>
struct get_value_type<std::back_insert_iterator<Container>> {
  using type = typename Container::value_type;
};

template <typename Char, typename Traits>
struct get_value_type<std::ostreambuf_iterator<Char, Traits>> {
  using type = Char;
};

template <typename T>
using get_value_type_t = typename get_value_type<T>::type;

template <typename Char, typename InputIt, typename OutputIt>
constexpr auto copy_str(InputIt it, InputIt end, OutputIt out) -> OutputIt {
  while (it != end) {
    *out++ = static_cast<Char>(*it++);
  }
  return out;
}

}  // namespace detail

/**
 * This class template is used to create a buffer around different iterator types.
 */
template <typename Iterator>
class iterator_buffer;

/**
 * This class fulfills the buffer API by using an output iterator and an internal cache.
 * @tparam Iterator The output iterator type.
 */
template <typename Iterator>
  requires(std::input_or_output_iterator<Iterator> &&
           std::output_iterator<Iterator, detail::get_value_type_t<Iterator>>)
class iterator_buffer<Iterator> final : public buffer<detail::get_value_type_t<Iterator>> {
 private:
  using char_t = detail::get_value_type_t<Iterator>;

 public:
  /**
   * Constructs and initializes the buffer with the given output iterator.
   * @param it The output iterator.
   */
  constexpr explicit iterator_buffer(Iterator it) : it_{it} {
    this->set_write_area(cache_);
  }

  iterator_buffer(const iterator_buffer&) = delete;
  iterator_buffer(iterator_buffer&&) = delete;
  iterator_buffer& operator=(const iterator_buffer&) = delete;
  iterator_buffer& operator=(iterator_buffer&&) = delete;

  /**
   * At destruction, the internal cache will be flushed to the output iterator.
   */
  constexpr ~iterator_buffer() override {
    flush();
  }

  /**
   * Flushes the internal cache to the output iterator.
   */
  constexpr void flush() noexcept {
    it_ = detail::copy_str<char_t>(cache_.data(), cache_.data() + this->get_used_count(), it_);
    this->set_write_area(cache_);
  }

  /**
   * Flushes and returns the output iterator at the next write position.
   * @return The output iterator.
   */
  constexpr Iterator out() noexcept {
    flush();
    return it_;
  }

 protected:
  constexpr result<std::span<char_t>> request_write_area(const size_t /*used*/, const size_t size) noexcept override {
    flush();
    const std::span<char_t> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  Iterator it_;
  std::array<char_t, detail::internal_buffer_size> cache_;
};

/**
 * This class fulfills the buffer API by using an output pointer.
 * @tparam Iterator The output iterator type.
 */
template <typename OutputPtr>
  requires(std::input_or_output_iterator<OutputPtr*> &&
           std::output_iterator<OutputPtr*, detail::get_value_type_t<OutputPtr*>>)
class iterator_buffer<OutputPtr*> final : public buffer<detail::get_value_type_t<OutputPtr*>> {
 public:
  /**
   * Constructs and initializes the buffer with the given output pointer.
   * @param ptr The output pointer.
   */
  constexpr explicit iterator_buffer(OutputPtr* ptr) : ptr_{ptr} {
    this->set_write_area({ptr, std::numeric_limits<size_t>::max()});
  }

  iterator_buffer(const iterator_buffer&) = delete;
  iterator_buffer(iterator_buffer&&) = delete;
  iterator_buffer& operator=(const iterator_buffer&) = delete;
  iterator_buffer& operator=(iterator_buffer&&) = delete;

  ~iterator_buffer() override = default;

  /**
   * Does nothing. Kept for uniformity with other iterator_buffer implementations.
   */
  constexpr void flush() noexcept {
    // Nothing.
  }

  /**
   * Returns the output pointer at the next write position.
   * @return The output pointer.
   */
  constexpr OutputPtr* out() {
    return ptr_ + this->get_used_count();
  }

 private:
  OutputPtr* ptr_;
};

/**
 * This class fulfills the buffer API by using the container of an contiguous back-insert iterator.
 * @tparam Container The container type of the back-insert iterator.
 */
template <typename Container>
  requires std::contiguous_iterator<typename Container::iterator>
class iterator_buffer<std::back_insert_iterator<Container>> final : public buffer<typename Container::value_type> {
 private:
  using char_t = typename Container::value_type;

 public:
  /**
   * Constructs and initializes the buffer with the given back-insert iterator.
   * @param it The output iterator.
   */
  constexpr explicit iterator_buffer(std::back_insert_iterator<Container> it) : container_{detail::get_container(it)} {
    static_cast<void>(request_write_area(0, std::min(container_.capacity(), detail::internal_buffer_size)));
  }

  iterator_buffer(const iterator_buffer&) = delete;
  iterator_buffer(iterator_buffer&&) = delete;
  iterator_buffer& operator=(const iterator_buffer&) = delete;
  iterator_buffer& operator=(iterator_buffer&&) = delete;

  /**
   * At destruction, the back-insert iterator will be flushed.
   */
  constexpr ~iterator_buffer() override {
    flush();
  }

  /**
   * Flushes the back-insert iterator by adjusting the size.
   */
  constexpr void flush() noexcept {
    container_.resize(used_ + this->get_used_count());
  }

  /**
   * Flushes and returns the back-insert iterator.
   * @return The back-insert iterator.
   */
  constexpr std::back_insert_iterator<Container> out() {
    flush();
    return std::back_inserter(container_);
  }

 protected:
  constexpr result<std::span<char_t>> request_write_area(const size_t used, const size_t size) noexcept override {
    const size_t new_size = container_.size() + size;
    container_.resize(new_size);
    used_ += used;
    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic): Performance in debug.
    const std::span<char_t> area{container_.data() + used_, new_size};
    this->set_write_area(area);
    return area.subspan(0, size);
  }

 private:
  size_t used_{};
  Container& container_;
};

template <typename Iterator>
iterator_buffer(Iterator&&) -> iterator_buffer<std::decay_t<Iterator>>;

namespace detail {

/**
 * A buffer that counts the number of code points written. Discards the output.
 * @tparam Char The used character type.
 */
template <typename Char>
class basic_counting_buffer final : public buffer<Char> {
 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): Can be left uninitialized.
  constexpr basic_counting_buffer() = default;
  constexpr basic_counting_buffer(const basic_counting_buffer&) = delete;
  constexpr basic_counting_buffer(basic_counting_buffer&&) noexcept = delete;
  constexpr basic_counting_buffer& operator=(const basic_counting_buffer&) = delete;
  constexpr basic_counting_buffer& operator=(basic_counting_buffer&&) noexcept = delete;
  constexpr ~basic_counting_buffer() override = default;

  /**
   * Calculates the number of code points that were written.
   * @return The number of code points.
   */
  [[nodiscard]] constexpr size_t count() const noexcept {
    return used_ + this->get_used_count();
  }

 protected:
  constexpr result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept override {
    used_ += used;
    const std::span<Char> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  size_t used_{};
  std::array<Char, detail::internal_buffer_size> cache_;
};

}  // namespace detail

}  // namespace emio