#pragma once
#include "style.hpp"

namespace tui {

// Cell is one visible character in the terminal with its style.
struct Cell {
    char32_t rune  = U' ';
    Style    style = StyleClear;
};

// A blank cell using terminal defaults.
constexpr Cell CellClear{U' ', StyleClear};

// Build a cell from a character and optional style.
inline Cell newCell(char32_t r, Style s = StyleClear) {
    return Cell{r, s};
}

} // namespace tui
