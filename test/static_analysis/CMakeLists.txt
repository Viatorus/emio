cmake_minimum_required(VERSION 3.14)

project(emioStaticAnalysis LANGUAGES CXX)

include(${CMAKE_SOURCE_DIR}/cmake/project-is-top-level.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/folders.cmake)

if (PROJECT_IS_TOP_LEVEL)
    find_package(emio REQUIRED)
endif ()

add_executable(emio_static_analysis test_main.cpp)
target_link_libraries(emio_static_analysis emio::emio)
target_compile_features(emio_static_analysis PRIVATE cxx_std_20)

add_folders(StaticAnalysis)
