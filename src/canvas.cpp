#include "tui/canvas.hpp"
#include <cstdlib>
#include <vector>

namespace tui {

// Each braille character encodes a 2×4 dot grid.
// BRAILLE[dotRow 0..3][dotCol 0..1] → bit to set.
// The Unicode braille block starts at U+2800 (empty cell).
static constexpr char32_t BRAILLE_OFFSET = U'⠀';

static constexpr char32_t BRAILLE[4][2] = {
    {0x01, 0x08},  // row 0 (top)
    {0x02, 0x10},  // row 1
    {0x04, 0x20},  // row 2
    {0x40, 0x80},  // row 3 (bottom)
};

static uint64_t cellKey(int x, int y) {
    return (static_cast<uint64_t>(x) << 32) | static_cast<uint32_t>(y);
}

void BrailleCanvas::setPoint(int bx, int by, Color color) {
    // Braille (bx, by) maps to terminal cell (bx/2, by/4).
    int tx = bx / 2;
    int ty = by / 4;
    char32_t bit = BRAILLE[by % 4][bx % 2];

    auto& cell = cells_[cellKey(tx, ty)];
    cell.bits |= bit;
    cell.color = color;
}

void BrailleCanvas::setLine(int bx0, int by0, int bx1, int by1, Color color) {
    for (auto [x, y] : linePoints(bx0, by0, bx1, by1))
        setPoint(x, y, color);
}

void BrailleCanvas::draw(Buffer& buf, Rect clipRect) const {
    for (auto& [k, bcell] : cells_) {
        int x = static_cast<int>(k >> 32);
        int y = static_cast<int>(static_cast<uint32_t>(k));
        Point p{x, y};
        if (!clipRect.contains(p)) continue;

        char32_t ch = BRAILLE_OFFSET + bcell.bits;
        buf.setCell(newCell(ch, newStyle(bcell.color)), p);
    }
}

// Straight-line iterator in braille coordinates.
// Walks from (x0,y0) to (x1,y1) one step at a time, left-to-right.
std::vector<std::pair<int,int>>
BrailleCanvas::linePoints(int x0, int y0, int x1, int y1) {
    std::vector<std::pair<int,int>> pts;

    // Always iterate left → right.
    if (x0 > x1) { std::swap(x0, x1); std::swap(y0, y1); }

    int dx = x1 - x0;
    int dy = std::abs(y1 - y0);
    int yDir = (y1 >= y0) ? 1 : -1;

    if (dx == 0) {
        // Vertical line
        for (int y = y0; y != y1 + yDir; y += yDir)
            pts.push_back({x0, y});
        return pts;
    }

    double slope = static_cast<double>(dy) / dx;
    double targetY = y0;
    int    currentY = y0;

    for (int x = x0; x <= x1; ++x) {
        pts.push_back({x, currentY});
        targetY += slope * yDir;
        while (currentY != static_cast<int>(targetY)) {
            pts.push_back({x, currentY});
            currentY += yDir;
        }
    }

    return pts;
}

} // namespace tui
