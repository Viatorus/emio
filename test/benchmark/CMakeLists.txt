cmake_minimum_required(VERSION 3.14)

project(emioBenchmarks LANGUAGES CXX)

include(${CMAKE_SOURCE_DIR}/cmake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(emio REQUIRED)
    enable_testing()
endif ()

add_executable(emio_benchmark
        bench_format.cpp
        bench_scan.cpp
        )

target_link_libraries(emio_benchmark
        Catch2::Catch2WithMain
        emio::emio
        fmt::fmt
        )

target_compile_features(emio_benchmark PRIVATE cxx_std_20)

add_test(NAME emio_benchmark COMMAND emio_benchmark)

add_folders(Benchmark)
