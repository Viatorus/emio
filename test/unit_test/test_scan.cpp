// Unit under test.
#include <emio/emio.hpp>

// Other includes.
#include <catch2/catch_test_macros.hpp>

namespace emio {

namespace detail::scan {

/**
 * This class represents a validated format string. The format string is either valid or not.
 * @note The validation happens at object construction.
 * @tparam Args The argument types to format.
 */
template <typename... Args>
class scan_string {
 public:
  /**
   * Constructs and validates the format string from any suitable char sequence at compile-time.
   * @note Terminates compilation if format string is invalid.
   * @param s The char sequence.
   */
  //  template <typename S>
  //    requires(std::is_constructible_v<std::string_view, S>)
  //  consteval scan_string(const S& s) noexcept {
  //    std::string_view str{s};
  //    if (validate_scan_string<Args...>(str)) {
  //      str_ = str;
  //    } else {
  //      // Invalid format string detected. Stop compilation.
  //      std::terminate();
  //    }
  //  }

  /**
   * Constructs and validates a runtime format string at runtime.
   * @param s The runtime format string.
   */
  constexpr scan_string(runtime_format_string s) noexcept {
    std::string_view str{s.view()};
    //    if (validate_scan_string<Args...>(str)) {
    str_ = str;
    //    }
  }

  /**
   * Returns the validated format string as view.
   * @return The view or invalid_format if the validation failed.
   */
  constexpr result<std::string_view> get() const noexcept {
    return str_;
  }

 private:
  result<std::string_view> str_{err::invalid_format};
};

struct scan_specs {};

template <typename Arg>
  requires(std::is_integral_v<Arg> && !std::is_same_v<Arg, bool> && !std::is_same_v<Arg, char>)
result<void> scan_arg(reader& in, const scan_specs&, Arg& arg) {
  EMIO_TRY(arg, in.parse_int<Arg>());
  return success;
}

template <typename Arg>
  requires(std::is_same_v<Arg, char>)
result<void> scan_arg(reader& in, const scan_specs&, Arg& arg) {
  EMIO_TRY(arg, in.read_char());
  return success;
}

template <typename T>
class scanner {
 public:
  result<void> parse(reader& rdr) noexcept {
    char c = rdr.read_char().assume_value();
    if (c == '}') {  // Format end.
      return success;
    }
    return err::invalid_format;
  }

  constexpr result<void> scan(reader& input, T& arg) const noexcept {
    return scan_arg(input, specs_, arg);
  }

  detail::scan::scan_specs specs_{};
};

class scan_parser : public parser_base<input_validation::enabled> {
 public:
  constexpr explicit scan_parser(reader& scan_rdr, std::string_view input) noexcept
      : parser_base<input_validation::enabled>{scan_rdr}, input_rdr_{input} {}

  constexpr result<void> process(char c) noexcept override {
    return input_rdr_.read_if_match_char(c);
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static): not possible because of template function
  constexpr result<void> find_and_scan_arg(uint8_t /*arg_pos*/) noexcept {
    return err::invalid_format;
  }

  template <typename Arg, typename... Args>
  constexpr result<void> find_and_scan_arg(uint8_t arg_pos, Arg& arg, Args&... args) noexcept {
    if (arg_pos == 0) {
      return scan_arg(arg);
    }
    return find_and_scan_arg(arg_pos - 1, args...);
  }

  template <typename T>
  static constexpr bool has_scanner_v = true;

 private:
  template <typename Arg>
  constexpr result<void> scan_arg(Arg& arg) noexcept {
    if constexpr (has_scanner_v<Arg>) {
      scanner<Arg> scanner;
      EMIO_TRYV(scanner.parse(this->format_rdr_));
      return scanner.scan(input_rdr_, arg);
      return success;
    } else {
      static_assert(has_scanner_v<Arg>,
                    "Cannot scan an argument. To make type T scannable provide a scanner<T> specialization.");
    }
  }

  reader input_rdr_;
};

template <typename... Args>
constexpr result<void> scan(std::string_view input, scan_string<Args...> scan_string, Args&... args) noexcept {
  EMIO_TRY(std::string_view str, scan_string.get());
  reader scan_rdr{str};
  scan_parser fh{scan_rdr, input};
  while (true) {
    uint8_t arg_nbr{detail::no_more_args};
    if (auto res = fh.parse(arg_nbr); !res) {
      return res.assume_error();
    }
    if (arg_nbr == detail::no_more_args) {
      break;
    }
    if (auto res = fh.find_and_scan_arg(arg_nbr, args...); !res) {
      return res.assume_error();
    }
  }
  return success;
}

}  // namespace detail::scan

template <typename... Args>
using scan_string = detail::scan::scan_string<std::type_identity_t<Args>...>;

// template <typename... Args>
//[[nodiscard]] detail::scan::scan_args_storage<sizeof...(Args)> make_scan_args(scan_string<Args...> format_str,
//                                                                               Args&... args) noexcept {
//   return {format_str.get(), args...};
// }

template <typename... Args>
result<void> scan(std::string_view input, scan_string<Args...> scan_string, Args&... args) {
  return detail::scan::scan(input, scan_string, args...);
}

}  // namespace emio

TEST_CASE("scan", "[scan]") {
  int a = 0;
  int b = 0;
  char c;
  emio::result<void> r = emio::scan("1,2!", emio::runtime("{},{}{}"), a, b, c);
  REQUIRE(r);
  CHECK(a == 1);
  CHECK(b == 2);
  CHECK(c == '!');
}