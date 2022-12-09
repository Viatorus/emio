# ---- Variables ----cd

get_filename_component(COMPILER_DIR ${CMAKE_CXX_COMPILER} DIRECTORY)

# We use variables separate from what CTest uses, because those have
# customization issues
set(
        SIZE_COVERAGE_TRACE_COMMAND
        python3 ${CMAKE_SOURCE_DIR}/cmake/elf_to_size_coverage.py
        --size-tool "${COMPILER_DIR}/${SIZE_TOOL}"
        -o "${PROJECT_BINARY_DIR}/size-coverage.info"
        ${SIZE_COVERAGE_FILES}
        CACHE STRING
        "; separated command to generate a trace for the 'size-coverage' target"
)

set(
        SIZE_COVERAGE_HTML_COMMAND
        genhtml --legend -f -q
        "${PROJECT_BINARY_DIR}/size-coverage.info"
        -p "${PROJECT_SOURCE_DIR}"
        -o "${PROJECT_BINARY_DIR}/size_coverage_html"
        CACHE STRING
        "; separated command to generate an HTML report for the 'size-coverage' target"
)

# ---- Coverage target ----

add_custom_target(
        size-coverage
        COMMAND ${SIZE_COVERAGE_TRACE_COMMAND}
        COMMAND ${SIZE_COVERAGE_HTML_COMMAND}
        COMMENT "Generating size-coverage report"
        VERBATIM
)
