# API

This is a small API overview. The public API is fully documented inside the source code. Unless otherwise stated,
everything can be used at compile-time.

The public namespace is `emio` only - no deeper nesting.

* [err](#err)
* [result](#result)
* [Buffer](#buffer)
    + [memory_buffer](#memory-buffer)
    + [span_buffer](#span-buffer)
    + [static_buffer](#static-buffer)
    + [iterator_buffer](#iterator-buffer)
    + [file_buffer](#file-buffer)
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

## Buffer

An abstract class which provides functionality for receiving a contiguous memory region to write into.

There exist multiple implementation of a buffer, all fulfilling a different use case.
Some buffers have an internal cache to provide a contiguous memory if the actually output object doesn't provide on.

### memory_buffer

- An endless growing buffer with an internal storage for small buffer optimization.

### span_buffer

- A buffer over a specific contiguous range.

### static_buffer

- A buffer containing a fixed-size storage.

### iterator_buffer

- A buffer for all kinds of output iterators (raw-pointers, back_insert_iterator or any other output iterator).
- The buffer's with a direct output iterator (e.g. std::string::iterator) do have an internal cache.

### file_buffer

- A buffer which over an std::File (file stream) with an internal cache.

## Reader

` class reader;`

- A class to read and parse a char sequence like a finite input stream.

*constructor(input)*

- Constructable from any suitable char sequence.

`peek() -> result<char>`

- Returns the next char without consuming it.

`read_char() -> result<char>`

- Returns one char.

`read_n_char(n) -> result<string_view>`

- Returns *n* chars.

`parse_int<T>(base = 10) -> result<T>`

- Parses an integer of type *T* with a specific *base*.

`read_until/_char/str/any_of/none_of(predicate, options) -> result<string_view>`

- Reads n chars until a given *predicate* (delimiter/group/function) applies.
- Has *options* to configure what should happen with the predicate and what should happen if EOF is reached.

`read_if_match_char/str(c/str)`

- Reads one/multiple chars if *c/str* matches the next char/chars.

## Writer

`class writer;`

- A class to write sequences of characters or other kinds of data into an output buffer.

*constructor(buffer)*

- Constructable from a reference to a buffer.

`write_char(c) -> result<void>`

- Writes a char *c* into the buffer.

`write_char_n(c, n) -> result<void>`

- Writes a char *c* *n* times into the buffer.

`write_char_escaped(c) -> result<void>`

- Writes a char *c* escaped into the buffer.

`write_str(sv) -> result<void>`

- Writes a char sequence *sv* into the buffer.

`write_str_escaped(sv) -> result<void>`

- Writes a char sequence *sv* escaped into the buffer.

`write_int(integer, options) -> result<void>`

- Writes an *integer* into the buffer.
- Has *options* to configure the base and if the alphanumerics should be in lower or upper case.

## Format

The following functions use a format string syntax which is nearly identical to the one used in
[fmt](https://fmt.dev/latest/syntax.html), which is similar to
[str.format](https://docs.python.org/3/library/stdtypes.html#str.format) in Python.

Things that are missing:

- chrono syntax (planned)
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

- Formats arguments according to the format string, and returns the result as a string.
- The return value depends on the type of the format string (valid-only type or not).

`format_to(out, format_str, ...args) -> result<Output>`

- Formats arguments according to the format string, and writes the result to the output.

`format_to_n(out, n, format_str, ...args) -> result<format_to_n_result<Output>>`

- Formats arguments according to the format string, and writes the result to the output iterator. At most *n* characters
  are written.

`formatted_size(format_str, ...args) -> size_t/result<size_t>`

- Determines the total number of characters in the formatted string by formatting args according to the format string.
- The return value depends on the type of the format string (valid-only type or not).

For each function there exists a function prefixed with v (e.g. `vformat`) which takes `format_args` instead of a
format string and arguments. The types are erased and can be used in non-template functions to reduce build-time, hide
implementations and reduce the binary size. **Note:** These type erased functions cannot be used at compile-time.

`format_args` can be created with:

`make_format_args(format_str, ...args) -> internal format_args_storage`

- Returns an object that stores a format string with an array of all arguments to format.
- Keep in mind that the storage uses reference semantics and does not extend the lifetime of args. It is the
  programmer's responsibility to ensure that args outlive the return value.

### Dynamic format specification

Unlike other libraries, the format specification cannot be changed through extra replacement fields, as it is possible
e.g. with fmt to dynamically set the precision to 1 with `fmt::format("{:.{}f}", 3.14, 1);`.

With emio it is possible to dynamically define _width_ and _precision_ through a `format_spec` object which is then
passed as an argument with the original value to the format function.

`format_spec{.width = <width>, .precision = <precision>}`

- If a spec is not defined inside the struct, the spec of the parsed format string will be applied.

In the example shown below the precision is set dynamically to 1:

```cpp
emio::format_spec spec{.precision = 1};
emio::format('{}', spec.with(3.14));  // 3.1
```

### Formatter

There exists formatter for builtin types like bool, char, string, integers, floats, void* and non-scoped enums, ranges
and tuple like types. Support for other standard types (e.g. chrono duration, optional) is planned.

Use `is_formattable_v<Type>` to check if a type is formattable.

A formatter exists of one optional function `validate` and two mandatory functions `parse` and `format`. If `validate`
is not present, `parse` must validate the format string.

A custom formatter for the class `foo` could be implemented like this:

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
   * @param format_rdr The reader of the format string.
   * @return Success if the format string is valid.
   */
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param format_rdr The reader of the format string.
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
- The return value depends on the type of the format spec string (valid-only type or not).

`print(file, format_str, ...args) -> result<void>`

- Formats arguments according to the format string, and writes the result to a file stream.

`println(format_str, ...args) -> void/result<void>`

- Formats arguments according to the format string, and writes the result to the standard output stream with a new
  line at the end.
- The return value depends on the type of the format string (valid-only type or not).

`println(file, format_str, ...args) -> result<void>`

- Formats arguments according to the format string, and writes the result to a file stream with a new line at the
  end.

For each function there exists a function prefixed with v (e.g. `vprint`) which allow the same functionality as
e.g. `vformat(...)` does for `format(...)`.

## Scan

The following functions use a format string syntax which is similar to the format syntax of `format`.

The grammar for the replacement field is the same. The grammar for the scan specific syntax is as follows:

```sass
format_spec ::=  ["#"][width][type]

type        ::=  "b" | "B" | "c" | "d" | "o" | "s" | "x" | "X"
```

`type`

- for integral types: the base to assume
    - b/B: base 2 (binary)
    - d: base 10 (decimal)
    - o: base 8 (octal)
    - x/X: base 16 (hexadecimal)
- c for char
- s for string/string_view

`#`

- for integral types: the alternate form
    - b/B: `0b` (e.g. 0b10110)
    - d: nothing (e.g. 9825)
    - o: leading `0` (e.g. 057)
    - x/X: `0x` (e.g 0x2fA3)
- if `#` is present but not the `type`, the base is deduced from the scanned alternate form.

The syntax of the format string is validated at compile-time. If a validation at runtime is required, the string must
be wrapped inside a `runtime_string` object. There is a simple helper function for that:

`runtime(string_view) -> runtime_string`

The API is structured as follows:

`scan(input, format_str, ...args) -> result<void>`

- Scans the input string for the given arguments according to the format string.

`scan_from(reader, format_str, ...args) -> result<void>`

- Scans the content of the reader for the given arguments according to the format string.

For each function there exists a function prefixed with v (e.g. `vscan`) which takes `scan_args` instead of a format
string and arguments. The types are erased and can be used in non-template functions to reduce build-time, hide
implementations and reduce the binary size. **Note:** These type erased functions cannot be used at compile-time.

`scan_args` can be created with:

`make_scan_args(format_str, ...args) -> internal scan_args_storage`

- Returns an object that stores a format string with an array of all arguments to scan.
- Keep in mind that the storage uses reference semantics and does not extend the lifetime of args. It is the
  programmer's responsibility to ensure that args outlive the return value.

### Scanner

There exists a scanner for builtin types like char and integers. Support for other types (e.g. string, float) is
planned.

Use `is_scanner_v<Type>` to check if a type is scannable.

A scanner exists of one optional function `validate` and two mandatory functions `parse` and `scan`. If `validate`
is not present, `parse` must validate the format string.

A custom scanner for the class `foo` could be implemented like this:

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
   * @param format_rdr The reader of the format string.
   * @return Success if the format string is valid.
   */
  static constexpr result<void> validate(reader& format_rdr) noexcept {
    return format_rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param format_rdr The reader of the format string.
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
