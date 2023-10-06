cmake_minimum_required(VERSION 3.14)

project(emio-fuzzing-format)

add_executable(format-fuzzer
        main.cpp
        )

target_include_directories(format-fuzzer PRIVATE ../../../include)
target_compile_features(format-fuzzer PRIVATE cxx_std_20)
target_compile_definitions(format-fuzzer PRIVATE -DEMIO_ENABLE_DEV_ASSERT)
