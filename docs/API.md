# API

This is a small API overview. The public API is fully documented inside the source code. Unless otherwise stated,
everything can be used at compile-time.

The public namespace is `emio` only - no deeper nesting.

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
Some buffers have an internal cache to provide a contiguous memory if the actually output object doesn't provide on.

### memory_buffer

- Endless growing buffer.

### span_buffer

- A buffer over a specific contiguous range.

### iterator_buffer

- A buffer for all kinds of output iterators (raw-pointers, back_insert_iterator or any other output iterator).
- The buffer's with a direct output iterator (e.g. std::string::iterator) do have an internal cache.

### file_buffer

- A buffer which over an std::File (file stream) with an internal cache.

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
- range and chrono syntax (planned)
- using an identifier as arg_id: `fmt::format("{nbr}", fmt::arg("nbr", 42)` (TBD)
- `'L'` options for locale (somehow possible but not with std::locale)

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

The format string syntax is validated at compile-time. If a runtime format string is required, the string must be
wrapped inside a `runtime` object.
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

#### Dynamic format specification

Unlike other libraries, the format specification cannot be changed through extra replacement fields like it is e.g.
possible with fmt to dynamically set the precision to 1 with `fmt::format("{:.{}f}", 3.14, 1);`.

With emio it is possible to dynamically define _width_ and _precision_ through a `format_spec` object which is than
passed as argument with the original value to the format function.

`format_spec{.width = <width>, .precision = <precision>}`

- If a spec is not defined in the struct, the spec of the parsed format string will be applied.

In the example shown below the precision is set dynamically to 1:

```cpp
emio::format_spec spec{.precision = 1};
emio::format('{}', spec.with(3.14));  // 3.1
```

### Formatter

There exists formatter for builtin types like bool, char, string, integers, floats, void* and non-scoped enums, ranges
and tuple
like types.
Support for other standard types (e.g. chrono duration, optional) is planned.

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

If the `validate` (or if absent the `parse`) function is not constexpr a runtime format strings must be used. The
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
- The return value depends on the type of the format string (valid-only type or not).

`print(file, format_str, ...args) -> result<void>`

- Formats arguments according to the format string, and writes the result to a file stream.

`println(format_str, ...args) -> void/result<void>`

- Formats arguments according to the format string, and writes the result to the standard output stream with a new line
  at the end.
- The return value depends on the type of the format string (valid-only type or not).

`println(file, format_str, ...args) -> result<void>`

- Formats arguments according to the format string, and writes the result to a file stream with a new line at the end.

For each function there exists a function prefixed with v (e.g. `vprint`) which allow the same functionality as
e.g. `vformat(...)` does for `format(...)`.