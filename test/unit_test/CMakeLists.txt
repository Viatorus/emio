cmake_minimum_required(VERSION 3.14)

project(emioTests LANGUAGES CXX)

include(${CMAKE_SOURCE_DIR}/cmake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(emio REQUIRED)
    enable_testing()
endif ()

add_executable(emio_test
        detail/test_bignum.cpp
        detail/test_bitset.cpp
        detail/test_conversion.cpp
        detail/test_ct_vector.cpp
        detail/test_decode.cpp
        detail/test_dragon.cpp
        detail/test_utf.cpp
        test_buffer.cpp
        test_dynamic_format_spec.cpp
        test_format.cpp
        test_format_api.cpp
        test_format_as.cpp
        test_format_emio_vs_fmt.cpp
        test_format_string.cpp
        test_format_to_api.cpp
        test_format_could_fail_api.cpp
        test_format_to_n_api.cpp
        test_formatted_size.cpp
        test_formatter.cpp
        test_iterator.cpp
        test_print.cpp
        test_ranges.cpp
        test_reader.cpp
        test_result.cpp
        test_scan.cpp
        test_std.cpp
        test_writer.cpp
)

target_link_libraries(emio_test
        Catch2::Catch2WithMain
        emio::emio
        fmt::fmt
)

target_compile_features(emio_test PRIVATE cxx_std_20)

target_compile_definitions(emio_test PRIVATE
        EMIO_ENABLE_DEV_ASSERT
)

add_test(NAME emio_test COMMAND emio_test)

add_folders(UnitTest)
