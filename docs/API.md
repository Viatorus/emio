# API

This is a small API overview. The public API is fully documented inside the source code. Unless otherwise stated,
everything can be used at compile-time.

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
- The format string is invalid (e.g. missing arguments).

`to_string(err) -> string_view`
- Returns the name of the error code.

## Result

`template<typename T> class result;`
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

### string_buffer
- Endless growing buffer.

### span_buffer
- Buffer over a specific contiguous range.

### iterator_buffer
- A buffer for all kinds of output iterators (raw-pointers, back_insert_iterator or any other output iterator).

## Reader

`template<typename Char> class reader;`
- A class to read and parse any char sequence like a finite input stream.

*constructor(input)*
- Constructable from any suitable char sequence.

`peek() -> result<char>`
- Returns the next char without consuming it.

`read_char() -> result<char>`
- Returns one char.

`read_n_char(n) -> result<view_t>`
- Returns *n* chars.

`parse_int<T>(base = 10) -> result<T>`
- Parses an integer of type *T* with a specific *base*.

`read_until/_char/str/any_of/none_of(predicate, options) -> result<view_t>`
- Reads n chars until a given *predicate* (delimiter/group/function) applies.
- Has *options* to configure what should happen with the predicate and what should happen if EOF is reached.

`read_if_match_char/str(c/str)`
- Reads one/multiple chars if *c/str* matches the next char/chars.

## Writer

`template<typename Char> class writer;`
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
- UTF-8 support (planned)
- floating-point, range and chrono syntax (planned)
- dynamic width and precision (planned but not via format string syntax)
- using an identifier as arg_id: `fmt::format("{nbr}", fmt::arg("nbr", 42)` (TBD)
- `'L'` options for locale (not planned)

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
type        ::=  "b" | "B" | "c" | "d" | "o" | "s" | "x" | "X"
```

The format string syntax is validated at compile-time. If a runtime format string is required, the string must be
wrapped inside a `runtime` object.

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

### Formatter

There exists formatter for builtin types like bool, char, string, integers, void* and non-scoped enums.
Support for other builtin types (e.g. floats), standard types (e.g. chrono duration) or ranges is planned.

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
   * Optional static function to validate the format spec for this type.
   * @note If not present, the parse function is invoked for validation.
   * @param rdr The format reader.
   * @return Success if the format spec is valid.
   */
  static constexpr result<void> validate(reader<char>& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to parse the format specs for this type.
   * @param rdr The format reader.
   * @return Success if the format spec is valid and could be parsed.
   */
  constexpr result<void> parse(reader<char>& rdr) noexcept {
    return rdr.read_if_match_char('}');
  }

  /**
   * Function to format the object of this type according to the parsed format specs.
   * @param wtr The output writer.
   * @param arg The argument to format.
   * @return Success if the formatting could be done.
   */
  constexpr result<void> format(writer<char>& wtr, const foo& arg) noexcept {
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
  constexpr result<void> format(writer<char>& wtr, const foo& arg) noexcept {
    return emio::format<int>(wtr, arg.x);
  }
};


int main() {
    emio::format("{:#x}", foo{42});  // 0x2a 
}
```
