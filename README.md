![logo](docs/res/logo.png)

[![Continuous Integration](https://github.com/Viatorus/emio/actions/workflows/ci.yml/badge.svg)](https://github.com/Viatorus/emio/actions/workflows/ci.yml)
[![codecov](https://codecov.io/gh/Viatorus/emio/branch/main/graph/badge.svg?token=7BQFK1PNLX)](https://codecov.io/gh/Viatorus/emio)

**em{io}** is a safe and fast high-level and low-level character input/output library for bare-metal and RTOS based
embedded systems with a very small binary footprint.

```cpp
std::string str = emio::format("The answer is {}.", 42);  

emio::result<std::string> res = emio::format(emio::runtime{"The answer is {}."}, 42);
if (res.has_value()) {
    std::string res = res.value();  // The answer is 42.
}

std::array<char, 128> storage;
emio::span_buffer buf{storage};
emio::format_to(buf, "The answer is {:#x}.", 42).value();
buf.view();  // The answer is 0x2a.

emio::writer wrt{buf};
wrt.write_str(" In decimal: ").value();
wrt.write_int(42).value();
wrt.write_char('.').value();
buf.view();  // The answer is 0x2a. In decimal: 42.

emio::reader rdr{"17c"};
EMIO_TRY(uint32_t number, rdr.parse_int<uint32_t>());  // 17
EMIO_TRY(char suffix, rdr.read_char());                // c
```

[**This library is in beta status! Please help to make it fly!**](https://github.com/Viatorus/emio/milestone/1)

* [API documentation](docs/API.md)
* Try em{io} [online](https://godbolt.org/z/Wo6xraWEW).

## Yet another character input/output library  

Bare-metal and RTOS based embedded systems do have special requirements which are mostly overlooked by the C++ standard,
its implementations and other libraries.

Therefore, this library:

* has a very small binary footprint **(42 times smaller than fmtlib!)**
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
- From [Conan Center]#(todo)

A compiler supporting C++20 is required. Tested with GCC 11.3 and Clang 14.

## Contributing

See the [CONTRIBUTING](docs/CONTRIBUTING.md) document.

## Licensing

em{io} is distributed under the [MIT license](LICENSE.md).
