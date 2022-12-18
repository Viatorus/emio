cmake_minimum_required(VERSION 3.14)

include(cmake/prelude.cmake)

project(
        emio
        VERSION 0.2.0
        DESCRIPTION "Character input/output library for embedded systems."
        HOMEPAGE_URL "https://github.com/viatorus/emio"
        LANGUAGES CXX
)

include(cmake/project-is-top-level.cmake)
include(cmake/variables.cmake)

# ---- Declare library ----

add_library(emio_emio INTERFACE)
add_library(emio::emio ALIAS emio_emio)

set_property(
        TARGET emio_emio PROPERTY
        EXPORT_NAME emio
)

target_include_directories(
        emio_emio ${warning_guard}
        INTERFACE
        "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)

target_compile_features(emio_emio INTERFACE cxx_std_20)

# ---- Install rules ----

if (NOT CMAKE_SKIP_INSTALL_RULES)
    include(cmake/install-rules.cmake)
endif ()

# ---- Developer mode ----

if (NOT emio_DEVELOPER_MODE)
    return()
elseif (NOT PROJECT_IS_TOP_LEVEL)
    message(
            AUTHOR_WARNING
            "Developer mode is intended for developers of emio"
    )
endif ()

include(cmake/dev-mode.cmake)
