# API

This is a small API overview. The public API is fully documented inside the source code. Unless otherwise stated,
everything can be used at compile-time.

The public namespace is `emio` only - no deeper nesting.

* [err](#err)
* [result](#result)
* [Buffer](#buffer)
    + [memory_buffer](#memorybuffer)
    + [span_buffer](#spanbuffer)
    + [static_buffer](#staticbuffer)
    + [iterator_buffer](#iteratorbuffer)
    + [file_buffer](#filebuffer)
    + [truncating_buffer](#truncatingbuffer)
* [Reader](#reader)
* [Writer](#writer)
* [Format](#format)
    + [Dynamic format specification](#dynamic-format-specification)
    + [Formatter](#formatter)
* [Print](#print)
* [Scan](#scan)
    + [Scanner](#scanner)

## err

A list of possible I/O errors as enum.
Every function describes the possible errors which can occur. See the source code documentation for more information.

`eof`

- End of file (e.g. reaching the end of an output array).

`invalid_argument`

- A parameter is incorrect (e.g. the output base is invalid).

`invalid_data`

- The data is malformed (e.g. no digit where a digit was expected).

`out_of_range`

- The parsed value is not in the range representable by the type (e.g. parsing 578 as uint8_t).

`invalid_format`

- The format string is invalid (e.g. missing arguments, wrong syntax).

`to_string(err) -> string_view`

- Returns the name of the error code.

*Example*

```cpp
std::string_view error_msg = emio::to_string(emio::err::invalid_format);  // invalid_format
```

## result

`template <typename T> class result;`

- The return type of almost all functions to propagate a value of type `T` on success or an error of type `emio::err`
  on failure.

*constructor(arg)*

- Constructable either from T or `emio::err`.

`has_value() -> bool`

- Checks whether the object holds a value.

`has_error() -> bool`

- Checks whether the object holds an error.

`value() -> T`

- Returns the value or throws/terminates if no value is held.

`value_or(T) -> T`

- Returns the value or returns the passed alternative if no value is held.

`assume_value() -> T`

- Returns the value without any checks. Invokes undefined behavior if no value is held.

`error() -> emio::err`

- Returns the error or throws/terminates if no error is held.

`assume_error() -> emio::err`

- Returns the error without any checks. Invokes undefined behavior if no error is held.

There exists two helper macros to simplify the control flow:

`EMIO_TRYV(expr)`

- Evaluates an expression *expr*. If successful, continues the execution. If unsuccessful, immediately returns from the
  calling function.

`EMIO_TRY(var, expr)`

- Evaluates an expression *expr*. If successful, assigns the value to a declaration *var*. If unsuccessful, immediately
  returns from the calling function.

*Example*

```cpp
result<int> parse_one(std::string_view sv) {
    if (sv.empty()) {
        return emio::err::eof;
    }
    if (sv == "1") {
        return 1;
    } else {
        return emio::err::invalid_data;
    }
}

emio::result<void> parse(std::string_view sv) {
    EMIO_TRY(int val, parse_one(sv));
    
    emio::result<int> res = parse_one(sv);
    if (!res) {
        return res.error();
    }
    if (res.assume_value() == val) {
        return emio::success;
    }
    return emio::err::invalid_format;
}
```

## Buffer

An abstract class which provides functionality for receiving a contiguous memory region of chars to write into.

There exist multiple implementation of a buffer, all fulfilling a different use case.
Some buffers have an internal cache to provide a contiguous memory if the actually output object doesn't provide on.

Additionally, some buffers can be reset to reuse the total capacity of the storage for the next operation.
This invalidates any obtaining view!

### memory_buffer

- An endless growing buffer with an internal storage for small buffer optimization.

*Example*

```cpp
emio::memory_buffer buf;

emio::result<std::span<char>> area = buf.get_write_area_of(50);
assert(area);
std::string_view view = buf.view();
std::string str = buf.str();
buf.reset();
```

### span_buffer

- A buffer over a specific contiguous range.

*Example*

```cpp
std::array<char, 512> storage;
emio::span_buffer buf{storage};

emio::result<std::span<char>> area = buf.get_write_area_of(50);
assert(area);
std::string_view view = buf.view();
std::string str = buf.str();
buf.reset();
```

### static_buffer

- A buffer containing a fixed-size storage.

*Example*

```cpp
emio::static_buffer<512> buf{storage}; 

emio::result<std::span<char>> area = buf.get_write_area_of(50);
assert(area);
std::string_view view = buf.view();
std::string str = buf.str();
buf.reset();
```

### iterator_buffer

- A buffer for all kinds of output iterators (raw-pointers, back_insert_iterator or any other output iterator).
- The buffer's with a direct output iterator (e.g. std::string::iterator) do have an internal cache.

*Example*

```cpp
std::string storage{"filled with something up"};
emio::iterator_buffer buf{std::back_inserter(storage)}; 

emio::result<std::span<char>> area = buf.get_write_area_of(50);
assert(area);
assert(buf.flush());
std::back_insert_iterator<std::string> out = buf.out();
```

### file_buffer

- A buffer over an std::File (file stream) with an internal cache.

*Example*

```cpp
std::FILE* file = std::fopen("test", "w");
emio::file_buffer buf{file}; 

emio::result<std::span<char>> area = buf.get_write_area_of(50);
assert(area);
assert(buf.flush());
buf.reset();
```

### truncating_buffer

- A buffer which truncates the remaining output if the limit of another provided buffer is reached.

*Example*

```cpp
emio::static_buffer<48> primary_buf{};
emio::truncating_buffer buf{primary_buf, 32};

emio::result<std::span<char>> area = buf.get_write_area_of(50);
assert(area);
assert(buf.flush());
assert(primary_buf.view().size() == 32);  // Only 32 bytes are flushed.
```

## Reader

` class reader;`

- A class to read and parse a char sequence like a finite input stream.

*constructor(input)*

- Constructable from any suitable char sequence.

*Example*

```cpp
emio::reader input{"1"};
assert(input.cnt_remaining() == 1);
assert(input.view_remaining() == "1");

std::string foo{"foo"};
emio::reader input2{foo};
assert(input2.cnt_remaining() == 3);
assert(input2.view_remaining() == "foo");
```

`peek() -> result<char>`

- Returns the next char without consuming it.

*Example*

```cpp
emio::reader input{"abc"};
emio::result<char> res = input.peek();
assert(res == 'a');
```

`read_char() -> result<char>`

- Returns one char.

*Example*

```cpp
emio::reader input{"abc"};
emio::result<char> res = input.read_char();
assert(res == 'a');
```

`read_n_char(n) -> result<string_view>`

- Returns *n* chars.

*Example*

```cpp
emio::reader input{"abc"};
emio::result<std::string_view> res = input.read_n_char(3);
assert(res == "abc");
```

`parse_int<T>(base = 10) -> result<T>`

- Parses an integer of type *T* with a specific *base*.

*Example*

```cpp
emio::reader input{"abc"};
emio::result<int> res = input.read_int(16 /* hexadecimal */);
assert(res == 0xabc);
```

`read_until/_char/str/any_of/none_of/([predicate,] options) -> result<string_view>`

- Reads n chars until a given *predicate* (delimiter/group/function) applies.
- Has *options* to configure what should happen with the predicate and what should happen if EOF is reached.

*Example*

```cpp
emio::reader get_input() {
    return emio::reader{"abc"};
}

// read_until_char
emio::result<std::string_view> res = get_input().read_until_char('c');
assert(res == "ab");

// read_until_str
emio::result<std::string_view> res = get_input().read_until_str("bc");
assert(res == "a");

// read_until_any_of
emio::result<std::string_view> res = get_input().read_until_any_of("cd");
assert(res == "ab");

// read_until_none_of
emio::result<std::string_view> res = get_input().read_until_none_of("ab");
assert(res == "ab");

// read_until with predicate
emio::result<std::string_view> res = get_input().read_until([](char c) { return c != 'a';});
assert(res == "a");
```

```cpp
// Different options.
emio::reader input{"abc 123 Hello"};

// with include_delimiter option 
emio::result<std::string_view> res = input.read_until_str(" ", {.include_delimiter = true});
assert(res == "abc ");

// with keep keep_delimiter option 
emio::result<std::string_view> res = input.read_until_str("Hello", {.keep_delimiter = true});
assert(res == "123 ");

//  with ignore_eof option 
emio::result<std::string_view> res = input.read_until_str("xyz", {.ignore_eof = true});
assert(res == "Hello");
```

`read_if_match_char/str(c/str) -> result<char/std::string_view>`

- Reads one/multiple chars if *c/str* matches the next char/chars.

*Example*

```cpp
emio::reader input{"abc"};
if (input.read_if_match_char('a')) {
  emio::result<std::string_view> res = input.read_if_match_str("bc");   
  assert(res == "bc");   
}
```

## Writer

`class writer;`

- A class to write sequences of characters or other kinds of data into an output buffer.

*constructor(buffer)*

*Example*

```cpp
emio::static_buffer<128> buf;
emio::writer output{buf};
```

- Constructable from a reference to a buffer.

`write_char(c) -> result<void>`

- Writes a char *c* into the buffer.

*Example*

```cpp
emio::writer output{get_buffer()};
emio::result<void> res = output.write_char('a');  // Buffer contains "a"
assert(res);
```

`write_char_n(c, n) -> result<void>`

- Writes a char *c* *n* times into the buffer.

*Example*

```cpp
emio::writer output{get_buffer()};
emio::result<void> res = output.write_char_n('a', 5);  // Buffer contains "aaaaa"
assert(res);
```

`write_char_escaped(c) -> result<void>`

- Writes a char *c* escaped into the buffer.

*Example*

```cpp
emio::writer output{get_buffer()};
emio::result<void> res = output.write_char_escaped('\n', 5);  // Buffer contains "\\n"
assert(res);
```

`write_str(sv) -> result<void>`

- Writes a char sequence *sv* into the buffer.

*Example*

```cpp
emio::writer output{get_buffer()};
emio::result<void> res = output.write_str("Hello");  // Buffer contains "Hello"
assert(res);
```

`write_str_escaped(sv) -> result<void>`

- Writes a char sequence *sv* escaped into the buffer.

*Example*

```cpp
emio::writer output{get_buffer()};
emio::result<void> res = output.write_str("\t 'and'");  // Buffer contains "\\t \'and\'"
assert(res);
```

`write_int(integer, options) -> result<void>`

- Writes an *integer* into the buffer.
- Has *options* to configure the base and if the alphanumerics should be in lower or upper case.

*Example*

```cpp
emio::writer output{get_buffer()};
emio::result<void> res = output.write_int(15);  // Buffer contains "15"
assert(res);

res = output.write_int(15, {.base = 16, upper_case = true});  // Buffer contains "15F"
assert(res);
```

## Format

The following functions use a format string syntax which is nearly identical to the one used in
[fmt](https://fmt.dev/latest/syntax.html), which is similar to
[str.format](https://docs.python.org/3/library/stdtypes.html#str.format) in Python.

Things that are missing:

- chrono syntax (planned)
- 'a'/'A' for hexadecimal floating point format (TBD)
- UTF-8 support (TBD)
- using an identifier as arg_id: `fmt::format("{nbr}", fmt::arg("nbr", 42)` (TBD)
- `'L'` options for locale (somehow possible but not with std::locale because of the binary size)

The grammar for the replacement field is as follows:

```sass
replacement_field ::=  "{" [arg_id] [":" format_spec] "}"

arg_id            ::=  integer

integer           ::=  digit+

digit             ::=  "0"..."9"
```

The grammar for the format specification is as follows:

```sass
format_spec ::=  [[fill]align][sign]["#"]["0"][width][type]

fill        ::=  <a character other than '{' or '}'>

align       ::=  "<" | ">" | "^"

sign        ::=  "+" | "-" | " "

width       ::=  integer

type        ::=  "b" | "B" | "c" | "d" | "o" | "s" | "x" | "X" | "e" | "E" | "f" | "F" | "g" | "G"
```

The syntax of the format string is validated at compile-time. If a validation at runtime is required, the string
must be wrapped inside a `runtime_string` object. There is a simple helper function for that:

`runtime(string_view) -> runtime_string`

Some functions (like `format` or `formatted_size`) are further optimized (simplified) in their return type if the format
string is a valid-only format string that could be ensured at compile-time.

`format(format_str, ...args) -> string/result<string>`

*Example*

```cpp
std::string str = emio::format("Hello {}!", 42);
assert(str == "Hello 42!");

std::string format_str = "Good by {}!";
emio::result<std::string> res = emio::format(emio::runtime(format_str), 42);
assert(res == "Good by 42!");
```

- Formats arguments according to the format string, and returns the result as a string.
- The return value depends on the type of the format string (valid-only type or not).

`format_to(out, format_str, ...args) -> result<Output>`

- Formats arguments according to the format string, and writes the result to the output iterator/buffer.
  **Note** If a raw output pointer or simple output iterator is used, no range checking can take place!

*Example*

```cpp
std::string out;
out.resize(10);
emio::result<std::string::iterator> res = emio::format_to(out.begin(), "Hello {}!", 42);
assert(res);
assert(out == "Hello 42!");
```

`format_to_n(out, n, format_str, ...args) -> result<format_to_n_result<Output>>`

- Formats arguments according to the format string, and writes the result to the output iterator/buffer. At most *n*
  characters are written.

*Example*

```cpp
std::string out;
out.resize(10);
emio::result<emio::format_to_n_result<std::string::iterator>> res = 
        emio::format_to_n(out.begin(), 7, "Hello {}!", 42);
assert(res)
assert(res->out == "Hello 4");
assert(res->size == 7);
```

`formatted_size(format_str, ...args) -> size_t/result<size_t>`

- Determines the total number of characters in the formatted string by formatting args according to the format string.
- The return value depends on the type of the format string (valid-only type or not).

*Example*

```cpp
size_t size = emio::formatted_size("> {}", 42);
assert(size == 4);
```

For each function there exists a function prefixed with v (e.g. `vformat`) which takes `format_args` instead of a
format string and arguments. The types are erased and can be used in non-template functions to reduce build-time, hide
implementations and reduce the binary size. **Note:** These type erased functions cannot be used at compile-time.

`format_args` can be created with:

`make_format_args(format_str, ...args) -> internal format_args_storage`

- Returns an object that stores a format string with an array of all arguments to format.
- Keep in mind that the storage uses reference semantics and does not extend the lifetime of args. It is the
  programmer's responsibility to ensure that args outlive the return value.

*Example*

```cpp
emio::result<void> internal_info(const emio::format_args& args) {
    emio::memory_buffer buf;
    
    emio::writer out{buf};  // Prefix message.
    EMIO_TRYV(out.write_str("INFO: "));  
    
    EMIO_TRYV(emio::vformat_to(out, args));
    log_message(out.view());  // Forward result. 
    return emio::success;
}

template<typename...Args>
void log_info(emio::format_string<Args...> fmt, const Args&...args) {
    emio::result<void> res = internal_info(emio::make_format_args(fmt, args...));  // type-erasing takes place 
    res.value();  // Throw on any error.
}

void do_something(int i) {
    log_info("Do something started with {}.", i);
}

int main() {
    do_something(42);  // INFO: Do something started with 42. 
}
```

### Dynamic format specification

Unlike other libraries, the format specification cannot be changed through extra replacement fields, as it is possible
e.g. with fmt to dynamically set the precision to 1 with `fmt::format("{:.{}f}", 3.14, 1);`.

With emio it is possible to dynamically define _width_ and _precision_ through a `format_spec` object which is then
passed as an argument with the original value to the format function.

`format_spec{.width = <width>, .precision = <precision>}`

- If a spec is not defined inside the struct, the spec of the parsed format string will be applied.

*Example*

```cpp
emio::format_spec spec{.precision = 1};
emio::format("{}", spec.with(3.141592653));  // 3.1
```

### Formatter

There exists formatter for builtin types like bool, char, string, integers, floats, void* and non-scoped enums, ranges
and tuple like types. Support for other standard types (e.g. chrono duration, optional) is planned.

For formatting values of pointer-like types, simply use `emio::ptr(p)`.

*Example*

```cpp
int* value = get();
emio::format("{}", emio::ptr(value));
```

Use `is_formattable_v<Type>` to check if a type is formattable.

A formatter exists of one optional function `validate` and two mandatory functions `parse` and `format`. If `validate`
is not present, `parse` must validate the format string.

*Example*

```cpp
struct foo {
    int x;
};

template <>
class emio::formatter<foo> {
 public:
  /**
   * Optional static function to validate the format string syntax for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param format_rdr The reader over the format string.
   * @return Success if the format string is valid.
   */
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param format_rdr The reader over the format string.
   * @return Success if the format string is valid and could be parsed.
   */
  constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to format the object of this type according to the parsed format specs.
   * @param out The output writer.
   * @param arg The argument to format.
   * @return Success if the formatting could be done.
   */
  constexpr result<void> format(writer& out, const foo& arg) const noexcept {
    return wtr.write_int(arg.x);
  }
};

int main() {
    emio::format("{}", foo{42});  // 42 
}
```

It is also possible to reuse existing formatters via inheritance or composition.

*Example*

```cpp
struct foo {
    int x;
};

template <>
class emio::formatter<foo> : public emio::format<int> {
 public:
  constexpr result<void> format(writer& out, const foo& arg) noexcept {
    return emio::format<int>::format(wtr, arg.x);
  }
};


int main() {
    emio::format("{:#x}", foo{42});  // 0x2a 
}
```

If the `validate` (or if absent the `parse`) function is not constexpr, a runtime format strings must be used. The
`format` function don't need to be constexpr if the formatting shouldn't be done at compile-time.

For simple type formatting, like formatting an enum class to its underlying integer or to a string, the function
`format_as` could be provided. The function must be in the same namespace since ADL is used.

*Example*

```cpp
namespace foo {
    
enum class bar {
    foobar,
    barfoo
};

constexpr auto format_as(const bar& w) noexcept {
  return static_cast<std::underlying_type_t<bar>>(w);
}

}
```

## Print

It is possible to directly print to the standard output or other file streams.

`print(format_str, ...args) -> void/result<void>`

- Formats arguments according to the format string, and writes the result to the standard output stream.
- The return value depends on the type of the format string (valid-only type or not).

*Example*

```cpp
emio::print("{}!", 42);  // Outputs: "42!"

emio::result<void> res = emio::print(emio::runtime("{}!"), 42);  // Outputs: "42!"
assert(res);
```

`print(file, format_str, ...args) -> result<void>`

- Formats arguments according to the format string, and writes the result to a file stream.

*Example*

```cpp
emio::result<void> res = emio::print(stderr, "{}!", 42);  // Outputs: "42!" to stderr
assert(res);
```

`println(format_str, ...args) -> void/result<void>`

- Formats arguments according to the format string, and writes the result to the standard output stream with a new line
  at the end.
- The return value depends on the type of the format string (valid-only type or not).

*Example*

```cpp
emio::println("{}!", 42);  // Outputs: "42!" with a line break

emio::result<void> res = emio::println(emio::runtime("{}!"), 42);  // Outputs: "42!" with a line break
assert(res);
```

`println(file, format_str, ...args) -> result<void>`

- Formats arguments according to the format string, and writes the result to a file stream with a new line at the end.

*Example*

```cpp
emio::result<void> res = emio::println(stderr, "{}!", 42);  // Outputs: "42!" with a line break to stderr
assert(res);
```

For each function there exists a function prefixed with v (e.g. `vprint`) which allow the same functionality as
e.g. `vformat(...)` does for `format(...)`.

## Scan

The following functions use a format string syntax which is similar to the format syntax of `format`.

The grammar for the replacement field is the same. The grammar for the scan specific syntax is as follows:

```sass
format_spec ::=  ["#"][width][type]

type        ::=  "b" | "B" | "c" | "d" | "o" | "s" | "x" | "X"
```

`#`

- for integral types: the alternate form
    - b/B: `0b` (e.g. 0b10110)
    - d: nothing (e.g. 9825)
    - o: leading `0` (e.g. 057)
    - x/X: `0x` (e.g 0x2fA3)
- if `#` is present but not the `type`, the base is deduced from the scanned alternate form.

*Example*

```cpp
int i;
int j;
int k;
int l;
scan("0b101 101 0101 0x101", "{:#} {:#} {:#} {:#}", i, j, k, l);
assert(i == 0b101);
assert(j == 101);
assert(k == 0101);
assert(l == 0x101);
```

`width`

- specifies the number of characters to include when parsing an argument

*Example*

```cpp
int i;
std::string_view j;
int k;
scan("125673", "{:2}{:3}{}", i, j, k);
assert(i == 12);
assert(j == "567");
assert(k == 3);
```

`type`

- for integral types: the base to assume
    - b/B: base 2 (binary)
    - d: base 10 (decimal)
    - o: base 8 (octal)
    - x/X: base 16 (hexadecimal)
- c for char
- s for string/string_view

*Example*

```cpp
int i;
int j;
int k;
int l;
scan("101 101 101 101", "{:b} {:d} {:o} {:x}", i, j, k, l);
assert(i == 0b101);
assert(j == 101);
assert(k == 0101);
assert(l == 0x101);
```

The syntax of the format string is validated at compile-time. If a validation at runtime is required, the string must
be wrapped inside a `runtime_string` object. There is a simple helper function for that:

`runtime(string_view) -> runtime_string`

The API is structured as follows:

`scan(input, format_str, ...args) -> result<void>`

*Example*

```cpp
int32_t i;
uint32_t j;
emio::result<void> res = emio::scan("-1,2", "{},{}", i, j);
assert(res);
assert(i == -1);
assert(j == 2);
```

- Scans the input string for the given arguments according to the format string.

`scan_from(reader, format_str, ...args) -> result<void>`

- Scans the content of the reader for the given arguments according to the format string.

*Example*

```cpp
int32_t i;
uint32_t j;
emio::reader input{"-1,2..."};
emio::result<void> res = emio::scan_from(input, "{},{}", i, j);
assert(res);
assert(i == -1);
assert(j == 2);
assert(input.view_remaining() == "...");
```

For each function there exists a function prefixed with v (e.g. `vscan`) which takes `scan_args` instead of a format
string and arguments. The types are erased and can be used in non-template functions to reduce build-time, hide
implementations and reduce the binary size. **Note:** These type erased functions cannot be used at compile-time.

`scan_args` can be created with:

`make_scan_args(format_str, ...args) -> internal scan_args_storage`

- Returns an object that stores a format string with an array of all arguments to scan.
- Keep in mind that the storage uses reference semantics and does not extend the lifetime of args. It is the
  programmer's responsibility to ensure that args outlive the return value.

### Scanner

There exists scanner for builtin types like char, string and integers. Support for other types (e.g. float) is planned.

Use `is_scanner_v<Type>` to check if a type is scannable.

A scanner exists of one optional function `validate` and two mandatory functions `parse` and `scan`. If `validate`
is not present, `parse` must validate the format string.

*Example*

```cpp
struct foo {
    int x;
};

template <>
class emio::scanner<foo> {
 public:
  /**
   * Optional static function to validate the format string syntax for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param format_rdr The reader over the format string.
   * @return Success if the format string is valid.
   */
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param format_rdr The reader over the format string.
   * @return Success if the format string is valid and could be parsed.
   */
  constexpr result<void> parse(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to scan the object of this type according to the parsed format specs.
   * @param input The input reader.
   * @param arg The argument to scan.
   * @return Success if the scanning could be done.
   */
  constexpr result<void> scan(reader& input, foo& arg) const noexcept {
    EMIO_TRYV(input.read_int(arg.x));
    return success;
  }
};

int main() {
    foo f{};
    emio::scan("42", "{}", i);  // f.x == 42
}
```

It is also possible to reuse existing scanner via inheritance or composition.

*Example*

```cpp
struct foo {
    int x;
};

template <>
class emio::scanner<foo> : public emio::scanner<int> {
 public:
  constexpr result<void> scan(reader& input, foo& arg) const noexcept {
    return emio::scanner<int>::scan(input, arg.x);
  }
};

int main() {
    foo f{};
    emio::scan("0x2A", "{:x}", f);  // f.x == 42
}
```

If the `validate` (or if absent the `parse`) function is not constexpr, a runtime strings must be used. The `scan`
function don't need to be constexpr if the scanning shouldn't be done at compile-time.
