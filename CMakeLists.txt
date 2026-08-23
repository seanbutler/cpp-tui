cmake_minimum_required(VERSION 3.16)
project(tui-cpp VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find ncursesw (wide-char ncurses for Unicode support)
find_library(NCURSESW_LIB NAMES ncursesw ncurses REQUIRED)
find_path(NCURSES_INCLUDE_DIR NAMES ncurses.h curses.h REQUIRED)

message(STATUS "Found ncurses: ${NCURSESW_LIB}")

# ── Static library: libtui ──────────────────────────────────────────────────
add_library(tui STATIC
    src/buffer.cpp
    src/block.cpp
    src/terminal.cpp
    src/events.cpp
    src/canvas.cpp
    src/grid.cpp
    src/widgets/paragraph.cpp
    src/widgets/list.cpp
    src/widgets/barchart.cpp
    src/widgets/gauge.cpp
    src/widgets/plot.cpp
    src/widgets/table.cpp
    src/style_parser.cpp
    src/widgets/sparkline.cpp
    src/widgets/stacked_barchart.cpp
    src/widgets/tabs.cpp
    src/widgets/tree.cpp
    src/widgets/piechart.cpp
)

target_include_directories(tui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${NCURSES_INCLUDE_DIR}
)

target_link_libraries(tui PUBLIC ${NCURSESW_LIB})

# ── Examples ────────────────────────────────────────────────────────────────
foreach(EXAMPLE hello_world demo demo2 demo3 demo_mouse)
    add_executable(${EXAMPLE} examples/${EXAMPLE}.cpp)
    target_link_libraries(${EXAMPLE} PRIVATE tui)
    set_target_properties(${EXAMPLE} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
    )
endforeach()
