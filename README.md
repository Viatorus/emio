![logo](docs/res/logo.png)

[![Continuous Integration](https://github.com/Viatorus/emio/actions/workflows/ci.yml/badge.svg)](https://github.com/Viatorus/emio/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/Viatorus/emio/branch/main/graph/badge.svg?token=7BQFK1PNLX)](https://codecov.io/gh/Viatorus/emio)
![Conan Center](https://img.shields.io/conan/v/emio)

**em{io}** is a safe and fast high-level and low-level character input/output library for bare-metal and RTOS based
embedded systems with a very small binary footprint.

```cpp
// High-level
std::string str = emio::format("The answer is {}.", 42);  // Format argument.

int answer{};
emio::result<void> scan_res = emio::scan(str, "The answer is {}.", answer);  // Scan input string.
if (scan_res) {
    emio::print("The answer is {}.", answer);  // Output to console.
}

// Without using heap.
emio::static_buffer<128> buf{}; 
emio::format_to(buf, "The answer is {:#x}.", 42).value();
buf.view();  // <- The answer is 0x2a.

// Low-level
emio::writer wrt{buf};
wrt.write_str(" In decimal: ").value();
wrt.write_int(42).value();
wrt.write_char('.').value();
buf.view();  // <- The answer is 0x2a. In decimal: 42.

emio::reader rdr{"17c"};
EMIO_TRY(uint32_t number, rdr.parse_int<uint32_t>());  // <- 17
EMIO_TRY(char suffix, rdr.read_char());                // <- c
```

[**This library is in beta status! Please help to make it fly!**](https://github.com/Viatorus/emio/milestone/1)

* [API documentation](docs/API.md)
* Try em{io} [online](https://godbolt.org/z/TzTTjnKEr).

## Yet another character input/output library  

Bare-metal and RTOS based embedded systems do have special requirements which are mostly overlooked by the C++ standard,
its implementations and other libraries.

Therefore, this library:

* has a very small binary footprint **(~38 times smaller than fmtlib!)**
* returns a result object instead of throwing an exception
* provides a high-level and low-level API which can be used at compile-time

Read more about it in the [DESIGN](docs/DESIGN.md) document.

## Including emio in your project

- With CMake and fetch content

```cmake
FetchContent_Declare(
        emio
        GIT_TAG main
        GIT_REPOSITORY https://github.com/Viatorus/emio.git
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(emio)
```

- Download the [single header file](https://viatorus.github.io/emio/) generated with [Quom](https://github.com/Viatorus/quom)
- From [Conan Center](https://conan.io/center/recipes/emio)

A compiler supporting C++20 is required. Tested with GCC 11.3 and Clang 15.

## Contributing

See the [CONTRIBUTING](docs/CONTRIBUTING.md) document.

## Licensing

em{io} is distributed under the [MIT license](LICENSE.md).
