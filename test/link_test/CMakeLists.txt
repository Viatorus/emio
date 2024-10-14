cmake_minimum_required(VERSION 3.14)

project(emioLinkTests LANGUAGES CXX)

include(${CMAKE_SOURCE_DIR}/cmake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(emio REQUIRED)
    enable_testing()
endif ()

add_executable(emio_link compile.cpp)

target_link_libraries(emio_link
        emio::emio
)

target_compile_features(emio_link PRIVATE cxx_std_20)

add_folders(CompileTest)
