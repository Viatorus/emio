include(cmake/folders.cmake)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)
include(CTest)

if (BUILD_TESTING OR BUILD_SIZE_COVERAGE)
    set(CMAKE_MODULE_PATH
            CACHE INTERNAL
            FORCE
            )

    set(FMT_HEADERS
            CACHE INTERNAL
            FORCE
            )

    set(FMT_OS OFF
            CACHE INTERNAL
            "Path to downloaded Catch2 modules"
            FORCE
            )


    Include(FetchContent)
    FetchContent_Declare(
            fmt
            GIT_COMMIT d9bc5f1320332db9a4bf7e103b0813b94e369304#GIT_TAG 9.1.0
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
#            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(fmt)
endif ()

if (BUILD_TESTING)
    Include(FetchContent)
    FetchContent_Declare(
            Catch2
            GIT_TAG v3.1.0
            GIT_REPOSITORY https://github.com/catchorg/Catch2.git
            GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
endif ()


if (BUILD_TESTING)
    add_subdirectory(test/static_analysis)
    add_subdirectory(test/unit_test)
    add_subdirectory(test/benchmark)
endif ()

if (BUILD_SIZE_COVERAGE)
    add_subdirectory(test/size_test)
endif ()

option(ENABLE_COVERAGE "Enable coverage support separate from CTest's" OFF)
if (ENABLE_COVERAGE)
    include(cmake/coverage.cmake)
endif ()

include(cmake/lint-targets.cmake)

add_folders(Project)
