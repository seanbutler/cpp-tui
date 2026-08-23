#pragma once
#include <cstdint>
#include <array>

namespace tui {

// Box-drawing characters used for widget borders.
constexpr char32_t VERTICAL_LINE   = U'│';
constexpr char32_t HORIZONTAL_LINE = U'─';
constexpr char32_t TOP_LEFT        = U'┌';
constexpr char32_t TOP_RIGHT       = U'┐';
constexpr char32_t BOTTOM_LEFT     = U'└';
constexpr char32_t BOTTOM_RIGHT    = U'┘';

// Special indicator characters.
constexpr char32_t DOT        = U'•';
constexpr char32_t ELLIPSES   = U'…';
constexpr char32_t UP_ARROW   = U'▲';
constexpr char32_t DOWN_ARROW = U'▼';
constexpr char32_t COLLAPSED  = U'+';
constexpr char32_t EXPANDED   = U'−';

// Block characters for bar/sparkline charts (empty → full).
constexpr std::array<char32_t, 9> BARS = {
    U' ', U'▁', U'▂', U'▃', U'▄', U'▅', U'▆', U'▇', U'█'
};

// Shaded block characters (light → solid).
constexpr std::array<char32_t, 5> SHADED_BLOCKS = {
    U' ', U'░', U'▒', U'▓', U'█'
};

} // namespace tui
