cmake_minimum_required(VERSION 3.14)

project(emioSizeTests LANGUAGES CXX)

include(${CMAKE_SOURCE_DIR}/cmake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(emio REQUIRED)
endif ()

macro(create_size_test NAME)
    string(REPLACE "/" "_" TARGET "emio_size_test_${NAME}")

    add_executable(${TARGET}
            stubs.cpp
            ${NAME}
            )

    target_link_libraries(${TARGET} PRIVATE emio::emio fmt::fmt)
    target_compile_features(${TARGET} PRIVATE cxx_std_20)

    list(APPEND SIZE_COVERAGE_FILES ${PROJECT_BINARY_DIR}/${TARGET})
endmacro()

create_size_test(base.cpp) # Base file must be the first one.
create_size_test(emio/doFormat_a.cpp)
create_size_test(emio/format_int.cpp)
create_size_test(emio/format_all.cpp)
create_size_test(emio/format_all_and_extra.cpp)
create_size_test(emio/format_and_scan_all.cpp)
create_size_test(emio/format_and_scan_all_runtime.cpp)
create_size_test(emio/format_and_write_int.cpp)
create_size_test(emio/format_double.cpp)
create_size_test(emio/format_runtime.cpp)
create_size_test(emio/format_to.cpp)
create_size_test(emio/format_to_n.cpp)
create_size_test(emio/format_int_twice.cpp)
create_size_test(emio/scan_all.cpp)
create_size_test(emio/scan_int.cpp)
create_size_test(emio/write_int.cpp)

create_size_test(floff/floff.cpp)
create_size_test(floff/floff_small.cpp)

create_size_test(fmt/doFormat_a.cpp)
create_size_test(fmt/fmt_dragon.cpp)
create_size_test(fmt/fmt_grisu.cpp)
create_size_test(fmt/fmt_grisu_and_dragon.cpp)
create_size_test(fmt/format_int.cpp)
create_size_test(fmt/format_all.cpp)
create_size_test(fmt/format_all_and_extra.cpp)
create_size_test(fmt/format_runtime.cpp)
create_size_test(fmt/format_to.cpp)
create_size_test(fmt/format_to_n.cpp)
create_size_test(fmt/format_int_twice.cpp)
create_size_test(fmt/vformat.cpp)

create_size_test(std/locale.cpp)
create_size_test(std/snprintf_and_sscanf.cpp)
create_size_test(std/snprintf.cpp)
create_size_test(std/sscanf.cpp)
create_size_test(std/string.cpp)
create_size_test(std/string_stream.cpp)
create_size_test(std/to_chars.cpp)
create_size_test(std/to_string_double.cpp)
create_size_test(std/to_string_int.cpp)

option(ENABLE_SIZE_COVERAGE "Enable size-coverage support separate from CTest's" OFF)
if (ENABLE_SIZE_COVERAGE)
    include(${CMAKE_SOURCE_DIR}/cmake/size-coverage.cmake)
endif ()

add_folders(SizeTest)
