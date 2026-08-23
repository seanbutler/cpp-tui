#pragma once
#include "geometry.hpp"
#include "cell.hpp"
#include <unordered_map>
#include <string>

namespace tui {

// Buffer is the drawing surface for a widget.
// Widgets call setCell/fill/setString during their draw() method,
// and the terminal backend then flushes the buffer to the screen.
class Buffer {
public:
    Rect rect;

    explicit Buffer(Rect r);

    Cell getCell(Point p) const;
    void setCell(Cell c, Point p);

    // Fill a rectangle with the same cell (used for borders and bars).
    void fill(Cell c, Rect r);

    // Write a UTF-32 string left-to-right from point p.
    void setString(const std::u32string& s, Style style, Point p);

    // Convenience overload for ASCII/UTF-8 strings (converted internally).
    void setString(const std::string& s, Style style, Point p);

    // Iterate over all cells (used by the terminal backend).
    const std::unordered_map<uint64_t, Cell>& cells() const { return cells_; }

    // Convert (x, y) to the map key.
    static uint64_t key(int x, int y) {
        return (static_cast<uint64_t>(x) << 32) |
               static_cast<uint32_t>(y);
    }
    static uint64_t key(Point p) { return key(p.x, p.y); }

private:
    std::unordered_map<uint64_t, Cell> cells_;
};

} // namespace tui
