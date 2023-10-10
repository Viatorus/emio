cmake_minimum_required(VERSION 3.14)

project(emio-fuzzing-dragon4)

add_executable(dragon4-fuzzer
        main.cpp
        )

target_include_directories(dragon4-fuzzer PRIVATE ../../../include)
target_compile_features(dragon4-fuzzer PRIVATE cxx_std_20)
target_compile_definitions(dragon4-fuzzer PRIVATE -DEMIO_ENABLE_DEV_ASSERT)
target_link_libraries(dragon4-fuzzer PRIVATE ${CMAKE_SOURCE_DIR}/build/lib/librust_ref.so)
