cmake_minimum_required(VERSION 3.14.0)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR arm)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY) # do not link executable when testing the compiler.

set(TOOLCHAIN_BIN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/gcc-arm-none-eabi-10.3-2021.10/bin)

set(CMAKE_C_COMPILER ${TOOLCHAIN_BIN_PATH}/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN_BIN_PATH}/arm-none-eabi-g++)
set(CMAKE_AR ${TOOLCHAIN_BIN_PATH}/arm-none-eabi-gcc-ar)
set(CMAKE_RANLIB ${TOOLCHAIN_BIN_PATH}/arm-none-eabi-gcc-ranlib)
