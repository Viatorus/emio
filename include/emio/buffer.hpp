//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <iterator>
#include <limits>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>

#include "detail/ct_vector.hpp"
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
  virtual constexpr ~buffer() = default;

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
    // If there is enough remaining capacity in the current write area, return it.
    // Otherwise, request a new write area from the concrete implementation.
    // There is a special case for fixed size buffers. Since they cannot grow, they simply return the
    // remaining capacity or EOF, if hitting zero capacity.

    const size_t remaining_capacity = area_.size() - used_;
    if (remaining_capacity >= size || fixed_size_ == fixed_size::yes) {
      if (remaining_capacity == 0 && size != 0) {
        return err::eof;
      }
      const size_t max_size = std::min(remaining_capacity, size);
      const std::span<char> area = area_.subspan(used_, max_size);
      used_ += max_size;
      return area;
    }
    EMIO_TRY(const std::span<Char> area, request_write_area(used_, size));
    used_ += area.size();
    return area;
  }

 protected:
  /// Flag to indicate if the buffer's size is fixed and cannot grow.
  enum class fixed_size : bool { no, yes };

  /**
   * Constructs the buffer.
   * @brief fixed Flag to indicate if the buffer's size is fixed and cannot grow.
   */
  constexpr explicit buffer(fixed_size fixed = fixed_size::no) noexcept : fixed_size_{fixed} {}

  /**
   * Requests a write area of the given size from a subclass.
   * @param used Already written characters into the current write area.
   * @param size The requested size of a new write area.
   * @return The write area with the requested size as maximum on success or eof if no write area is available.
   */
  virtual constexpr result<std::span<Char>> request_write_area(const size_t used, const size_t size) noexcept {
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
  fixed_size fixed_size_{fixed_size::no};
  size_t used_{};
  std::span<Char> area_{};
};

/**
 * This class fulfills the buffer API by providing an endless growing buffer.
 * @tparam Char The character type.
 * @tparam StorageSize The size of the inlined storage for small buffer optimization.
 */
template <typename Char = char, size_t StorageSize = 32>
class memory_buffer final : public buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with the internal storage size.
   */
  constexpr memory_buffer() : memory_buffer{0} {}

  /**
   * Constructs and initializes the buffer with the given capacity.
   * @param capacity The initial capacity.
   */
  constexpr explicit memory_buffer(const size_t capacity) {
    // Request at least the internal storage size.
    static_cast<void>(request_write_area(0, std::max(vec_.capacity(), capacity)));
  }

  constexpr memory_buffer(const memory_buffer&) = default;
  constexpr memory_buffer(memory_buffer&&) noexcept = default;
  constexpr memory_buffer& operator=(const memory_buffer&) = default;
  constexpr memory_buffer& operator=(memory_buffer&&) noexcept = default;
  constexpr ~memory_buffer() override = default;

  /**
   * Obtains a view over the underlying string object.
   * @return The view.
   */
  [[nodiscard]] constexpr std::basic_string_view<Char> view() const noexcept {
    return {vec_.data(), used_ + this->get_used_count()};
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
    const size_t new_size = vec_.size() + size;
    vec_.reserve(new_size);
    used_ += used;
    const std::span<Char> area{vec_.data() + used_, size};
    this->set_write_area(area);
    return area;
  }

 private:
  size_t used_{};
  detail::ct_vector<Char, StorageSize> vec_{};
};

/**
 * This class fulfills the buffer API by using a span over an contiguous range.
 * @tparam Char The character type.
 */
template <typename Char = char>
class span_buffer : public buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with an empty span.
   */
  constexpr span_buffer() : buffer<Char>{buffer<Char>::fixed_size::yes} {};

  /**
   * Constructs and initializes the buffer with the given span.
   * @param span The span.
   */
  constexpr explicit span_buffer(const std::span<Char> span)
      : buffer<Char>{buffer<Char>::fixed_size::yes}, span_{span} {
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

/**
 * This class fulfills the buffer API by providing a fixed-size storage.
 * @tparam Char The character type.
 * @tparam StorageSize The size of the storage.
 */
template <typename Char, size_t StorageSize>
class static_buffer final : private std::array<Char, StorageSize>, public span_buffer<Char> {
 public:
  /**
   * Constructs and initializes the buffer with the storage.
   */
  constexpr static_buffer() noexcept : span_buffer<Char>{std::span{*this}} {}

  constexpr static_buffer(const static_buffer&) = default;
  constexpr static_buffer(static_buffer&&) noexcept = default;
  constexpr static_buffer& operator=(const static_buffer&) = default;
  constexpr static_buffer& operator=(static_buffer&&) noexcept = default;
  constexpr ~static_buffer() override = default;

  // Note: We inherit from std::array to put the storage lifetime before span_buffer.
  // Clang will otherwise complain if the storage is a member variable and used during compile-time.
};

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
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized
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

/**
 * This class fulfills the buffer API by using a file stream and an internal cache.
 */
class file_buffer : public buffer<char> {
 public:
  /**
   * Constructs and initializes the buffer with the given file stream.
   * @param file The file stream.
   */
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized
  constexpr explicit file_buffer(std::FILE* file) : file_{file} {
    this->set_write_area(cache_);
  }

  file_buffer(const file_buffer&) = delete;
  file_buffer(file_buffer&&) = delete;
  file_buffer& operator=(const file_buffer&) = delete;
  file_buffer& operator=(file_buffer&&) = delete;
  ~file_buffer() override = default;  // Doesn't flush because it could fail!

  /**
   * Flushes the internal cache to the file stream.
   * @note Does not flush the file stream itself!
   */
  result<void> flush() noexcept {
    const size_t written = std::fwrite(cache_.data(), sizeof(char), this->get_used_count(), file_);
    if (written != this->get_used_count()) {
      return err::eof;
    }
    this->set_write_area(cache_);
    return success;
  }

 protected:
  result<std::span<char>> request_write_area(const size_t /*used*/, const size_t size) noexcept override {
    EMIO_TRYV(flush());
    const std::span<char> area{cache_};
    this->set_write_area(area);
    if (size > cache_.size()) {
      return area;
    }
    return area.subspan(0, size);
  }

 private:
  std::FILE* file_;
  std::array<char, detail::internal_buffer_size> cache_;
};

namespace detail {

/**
 * A buffer that counts the number of code points written. Discards the output.
 * @tparam Char The used character type.
 */
template <typename Char>
class basic_counting_buffer final : public buffer<Char> {
 public:
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init): cache_ can be left uninitialized.
  constexpr basic_counting_buffer() noexcept = default;
  constexpr basic_counting_buffer(const basic_counting_buffer&) = delete;
  constexpr basic_counting_buffer(basic_counting_buffer&&) noexcept = delete;
  constexpr basic_counting_buffer& operator=(const basic_counting_buffer&) = delete;
  constexpr basic_counting_buffer& operator=(basic_counting_buffer&&) noexcept = delete;
  constexpr ~basic_counting_buffer() override = default;

  /**
   * Calculates the number of Char's that were written.
   * @return The number of Char's.
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
