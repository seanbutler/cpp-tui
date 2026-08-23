#pragma once
#include <cstdint>

namespace tui {

// Color is an xterm 256 color value.
// -1 means "terminal default" (no color set).
using Color = int;

constexpr Color ColorClear   = -1;
constexpr Color ColorBlack   = 0;
constexpr Color ColorRed     = 1;
constexpr Color ColorGreen   = 2;
constexpr Color ColorYellow  = 3;
constexpr Color ColorBlue    = 4;
constexpr Color ColorMagenta = 5;
constexpr Color ColorCyan    = 6;
constexpr Color ColorWhite   = 7;

// Modifier flags that can be OR'd together.
using Modifier = uint32_t;

constexpr Modifier ModifierClear     = 0;
constexpr Modifier ModifierBold      = 1 << 0;
constexpr Modifier ModifierUnderline = 1 << 1;
constexpr Modifier ModifierReverse   = 1 << 2;

// Style describes the visual appearance of a terminal cell.
struct Style {
    Color    fg  = ColorClear;
    Color    bg  = ColorClear;
    Modifier mod = ModifierClear;
};

// StyleClear is the zero-value style (terminal defaults everywhere).
constexpr Style StyleClear{ColorClear, ColorClear, ModifierClear};

// Helper to build a style concisely, like Go's NewStyle().
inline Style newStyle(Color fg,
                      Color bg  = ColorClear,
                      Modifier mod = ModifierClear) {
    return Style{fg, bg, mod};
}

} // namespace tui
