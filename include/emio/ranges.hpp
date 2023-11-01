//
// Copyright (c) 2023 - present, Toni Neubert
// All rights reserved.
//
// For the license information refer to emio.hpp
//
// Additional licence for this file:
// Copyright (c) 2018 - present, Remotion (Igor Schulz)

#pragma once

#include "detail/format/ranges.hpp"

namespace emio {

/**
 * Formatter for ranges.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_valid_range<T> && !detail::format::is_contiguous_but_not_span<T>)
class formatter<T> {
 public:
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    EMIO_TRY(char c, format_rdr.read_char());
    if (c == 'n') {
      EMIO_TRY(c, format_rdr.read_char());
    }
    if (c == '}') {
      return success;
    }
    if (c == ':') {
      return formatter<detail::format::element_type_t<T>>::validate(format_rdr);
    } else {
      return err::invalid_format;
    }
  }

  constexpr formatter() noexcept
    requires(!detail::format::is_map<T> && !detail::format::is_set<T>)
      : specs_{detail::sv("["), detail::sv("]"), detail::sv(", ")} {}

  constexpr formatter() noexcept
    requires(detail::format::is_map<T>)
      : specs_{detail::sv("{"), detail::sv("}"), detail::sv(", ")} {
    underlying_.set_brackets({}, {});
    underlying_.set_separator(detail::sv(": "));
  }

  constexpr formatter() noexcept
    requires(detail::format::is_set<T> && !detail::format::is_map<T>)
      : specs_{detail::sv("{"), detail::sv("}"), detail::sv(", ")} {}

  constexpr void set_separator(std::string_view separator) noexcept {
    specs_.separator = separator;
  }

  constexpr void set_brackets(std::string_view opening_bracket, std::string_view closing_bracket) noexcept {
    specs_.opening_bracket = opening_bracket;
    specs_.closing_bracket = closing_bracket;
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    char c = format_rdr.peek().assume_value();
    if (c == 'n') {
      set_brackets({}, {});
      format_rdr.pop();  // n
      c = format_rdr.peek().assume_value();
    }
    if (c == '}') {
      detail::format::maybe_set_debug_format(underlying_, true);
    } else {
      format_rdr.pop();  // :
    }
    return underlying_.parse(format_rdr);
  }

  constexpr result<void> format(writer& out, const T& arg) const noexcept {
    EMIO_TRYV(out.write_str(specs_.opening_bracket));

    using std::begin;
    using std::end;
    auto first = begin(arg);
    const auto last = end(arg);
    for (auto it = first; it != last; ++it) {
      if (it != first) {
        EMIO_TRYV(out.write_str(specs_.separator));
      }
      EMIO_TRYV(underlying_.format(out, *it));
    }
    EMIO_TRYV(out.write_str(specs_.closing_bracket));
    return success;
  }

  constexpr auto& underlying() noexcept {
    return underlying_;
  }

 private:
  formatter<detail::format::element_type_t<T>> underlying_{};
  detail::format::ranges_specs specs_{};
};

/**
 * Formatter for contiguous ranges.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_valid_range<T> && detail::format::is_contiguous_but_not_span<T>)
class formatter<T> : public formatter<std::span<const detail::format::element_type_t<T>>> {};

/**
 * Formatter for tuple like types.
 * @tparam T The type.
 */
template <typename T>
  requires(detail::format::is_valid_tuple<T>)
class formatter<T> {
 public:
  constexpr formatter() : specs_{detail::sv("("), detail::sv(")"), detail::sv(", ")} {}

  constexpr void set_separator(std::string_view separator) noexcept {
    specs_.separator = separator;
  }

  constexpr void set_brackets(std::string_view opening_bracket, std::string_view closing_bracket) noexcept {
    specs_.opening_bracket = opening_bracket;
    specs_.closing_bracket = closing_bracket;
  }

  static constexpr result<void> validate(reader& format_rdr) noexcept {
    EMIO_TRY(char c, format_rdr.read_char());
    if (c == 'n') {
      EMIO_TRY(c, format_rdr.read_char());
    }
    if (c == '}') {
      return success;
    }
    if (c == ':') {
      return validate_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), format_rdr);
    } else {
      return err::invalid_format;
    }
  }

  constexpr result<void> parse(reader& format_rdr) noexcept {
    char c = format_rdr.peek().assume_value();
    if (c == 'n') {
      set_brackets({}, {});
      format_rdr.pop();  // n
      c = format_rdr.peek().assume_value();
    }
    bool set_debug = false;
    if (c == '}') {
      set_debug = true;
    } else {
      format_rdr.pop();  // :
    }
    return parse_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), format_rdr, set_debug);
  }

  constexpr result<void> format(writer& out, const T& args) const noexcept {
    EMIO_TRYV(out.write_str(specs_.opening_bracket));
    EMIO_TRYV(format_for_each(std::make_index_sequence<std::tuple_size_v<T>>(), out, args));
    EMIO_TRYV(out.write_str(specs_.closing_bracket));
    return success;
  }

 private:
  template <size_t... Ns>
  static constexpr result<void> validate_for_each(std::index_sequence<Ns...> /*unused*/, reader& format_rdr) noexcept {
    size_t reader_pos = 0;
    result<void> res = success;
    const auto validate = [&reader_pos, &res](const auto type_identity, reader r /*copy!*/) noexcept {
      res = decltype(type_identity)::type::validate(r);
      if (res.has_error()) {
        return false;
      }
      if (reader_pos == 0) {
        reader_pos = r.pos();
      } else if (reader_pos != r.pos()) {
        res = err::invalid_format;
      }
      return res.has_value();
    };
    static_cast<void>(validate);  // Maybe unused warning.
    if ((validate(std::type_identity<std::tuple_element_t<Ns, detail::format::tuple_formatters<T>>>{}, format_rdr) &&
         ...) &&
        reader_pos != 0) {
      format_rdr.pop(reader_pos);
      return success;
    }
    return res;
  }

  static constexpr result<void> validate_for_each(std::index_sequence<> /*unused*/, reader& /*format_rdr*/) noexcept {
    return err::invalid_format;
  }

  template <size_t... Ns>
  constexpr result<void> parse_for_each(std::index_sequence<Ns...> /*unused*/, reader& format_rdr,
                                        const bool set_debug) noexcept {
    using std::get;

    size_t reader_pos = 0;
    result<void> res = success;
    const auto parse = [&reader_pos, &res, set_debug](auto& f, reader r /*copy!*/) noexcept {
      detail::format::maybe_set_debug_format(f, set_debug);
      res = f.parse(r);
      reader_pos = r.pos();
      return res.has_value();
    };
    static_cast<void>(parse);  // Maybe unused warning.
    if ((parse(get<Ns>(formatters_), format_rdr) && ...)) {
      format_rdr.pop(reader_pos);
      return success;
    }
    return res;
  }

  constexpr result<void> parse_for_each(std::index_sequence<> /*unused*/, reader& format_rdr,
                                        const bool set_debug) noexcept {
    if (set_debug) {
      format_rdr.pop();  // }
      return success;
    }
    return err::invalid_format;
  }

  template <size_t N, size_t... Ns>
  constexpr result<void> format_for_each(std::index_sequence<N, Ns...> /*unused*/, writer& out,
                                         const T& args) const noexcept {
    using std::get;
    EMIO_TRYV(get<N>(formatters_).format(out, get<N>(args)));

    result<void> res = success;
    const auto format = [&res, &out, this](auto& f, const auto& arg) noexcept {
      res = out.write_str(specs_.separator);
      if (res.has_error()) {
        return false;
      }
      res = f.format(out, arg);
      return res.has_value();
    };
    static_cast<void>(format);  // Maybe unused warning.
    if ((format(get<Ns>(formatters_), get<Ns>(args)) && ...)) {
      return success;
    }
    return res;
  }

  constexpr result<void> format_for_each(std::index_sequence<> /*unused*/, writer& /*out*/,
                                         const T& /*args*/) const noexcept {
    return success;
  }

  detail::format::tuple_formatters<T> formatters_{};
  detail::format::ranges_specs specs_{};
};

}  // namespace emio
