cmake_minimum_required(VERSION 3.14)

project(emioLinkTests LANGUAGES CXX)

include(${CMAKE_SOURCE_DIR}/cmake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(emio REQUIRED)
    enable_testing()
endif ()

add_executable(emio_compile compile.cpp)

target_link_libraries(emio_compile
        emio::emio
)

target_compile_features(emio_compile PRIVATE cxx_std_20)

add_executable(emio_compile_freestanding compile.cpp)

target_link_libraries(emio_compile_freestanding
        emio::emio
)

target_compile_features(emio_compile_freestanding PRIVATE cxx_std_20)
target_compile_options(emio_compile_freestanding PRIVATE -ffreestanding)

add_folders(CompileTest)
