#pragma once
#include "tui/geometry.hpp"
#include "tui/style.hpp"
#include "tui/buffer.hpp"
#include <vector>
#include <unordered_map>
#include <cstdint>

namespace tui {

// BrailleCanvas renders pixel-level dots using Unicode braille characters.
//
// Each braille character cell covers a 2-wide × 4-tall grid of dots.
// Mapping: BRAILLE[dotRow][dotCol] gives the bit to OR into the cell's
// code point, and the final character = U'⠀' + accumulated bits.
//
// This gives 2× horizontal and 4× vertical resolution versus normal text,
// making it ideal for line charts and scatter plots.
class BrailleCanvas {
public:
    // Set a single dot at braille coordinates (bx, by) with the given color.
    void setPoint(int bx, int by, Color color);

    // Draw a line between two braille-coordinate points.
    void setLine(int bx0, int by0, int bx1, int by1, Color color);

    // Write all accumulated cells into a Buffer, clipping to clipRect.
    void draw(Buffer& buf, Rect clipRect) const;

    void clear() { cells_.clear(); }

private:
    struct BrailleCell {
        char32_t bits  = 0;  // accumulated braille dot bits (before adding offset)
        Color    color = ColorWhite;
    };

    // Key = terminal (x,y) packed as a uint64.
    std::unordered_map<uint64_t, BrailleCell> cells_;

    // Bresenham-style line: returns all braille-coordinate points on the line.
    static std::vector<std::pair<int,int>> linePoints(int x0, int y0, int x1, int y1);
};

} // namespace tui
