//
// Copyright (c) 2021 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp

#pragma once

#include <iterator>

namespace emio {

/**
 * This class provides the member types for an output iterator.
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
class truncating_iterator_base {
 public:
  using iterator_category = std::output_iterator_tag;
  using value_type = typename std::iterator_traits<OutputIt>::value_type;
  using difference_type = std::ptrdiff_t;
  using pointer = void;
  using reference = void;

  /**
   * Default constructs a truncating iterator.
   */
  constexpr truncating_iterator_base() = default;

  /**
   * Constructs a truncating iterator with an output iterator to wrap and a given output limit.
   * @param out The output iterator.
   * @param limit The output limit.
   */
  constexpr truncating_iterator_base(OutputIt out, size_t limit)
      : out_{out}, limit_{static_cast<std::iter_difference_t<OutputIt>>(limit)} {}

  /**
   * Returns the actual output iterator.
   * @return The output iterator.
   */
  constexpr OutputIt out() const {
    return out_;
  }

  /**
   * Returns the count of the total (not truncated) outputted elements.
   * @return The count.
   */
  [[nodiscard]] constexpr std::iter_difference_t<OutputIt> count() const noexcept {
    return count_;
  }

 protected:
  OutputIt out_{};
  std::iter_difference_t<OutputIt> limit_{};
  std::iter_difference_t<OutputIt> count_{};
};

/**
 * The truncating iterator is an output iterator which limits the amount of elements passed to an wrapped output
 * iterator.
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
class truncating_iterator;

// CTAD guide.
template <typename OutputIt>
truncating_iterator(OutputIt&&, size_t) -> truncating_iterator<std::decay_t<OutputIt>>;

/**
 * Template specification for a pure output-only iterator (e.g. the back_insert_iterator).
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
  requires(std::is_void_v<typename std::iterator_traits<OutputIt>::value_type>)
class truncating_iterator<OutputIt> : public truncating_iterator_base<OutputIt> {
 public:
  using truncating_iterator_base<OutputIt>::truncating_iterator_base;

  template <typename T>
  constexpr truncating_iterator& operator=(T val) {
    if (this->count_ < this->limit_) {
      *this->out_++ = val;
    }
    this->count_ += 1;
    return *this;
  }

  constexpr truncating_iterator& operator++() {
    return *this;
  }
  constexpr truncating_iterator& operator++(int) {
    return *this;
  }
  constexpr truncating_iterator& operator*() {
    return *this;
  }
};

/**
 * Template specification for any other output iterator.
 * @tparam OutputIt The iterator type to wrap.
 */
template <typename OutputIt>
  requires(!std::is_void_v<typename std::iterator_traits<OutputIt>::value_type>)
class truncating_iterator<OutputIt> : public truncating_iterator_base<OutputIt> {
 public:
  using value_type = typename truncating_iterator_base<OutputIt>::value_type;

  using truncating_iterator_base<OutputIt>::truncating_iterator_base;

  constexpr truncating_iterator& operator++() {
    if (this->count_ < this->limit_) {
      ++this->out_;
    }
    this->count_ += 1;
    return *this;
  }

  constexpr truncating_iterator operator++(int) {
    auto it = *this;
    ++*this;
    return it;
  }

  constexpr value_type& operator*() const {
    if (this->count_ < this->limit_) {
      return *this->out_;
    }
    return black_hole_;
  }

 private:
  mutable value_type black_hole_{};
};

namespace detail {

template <typename Iterator>
struct get_value_type;

template <typename Iterator>
struct get_value_type<truncating_iterator<Iterator>> : get_value_type<Iterator> {};

}  // namespace detail

}  // namespace emio
