#include "tui/buffer.hpp"
#include "tui/utf8.hpp"

namespace tui {

Buffer::Buffer(Rect r) : rect(r) {
    // Pre-fill the entire rect with blank cells.
    fill(CellClear, r);
}

Cell Buffer::getCell(Point p) const {
    auto it = cells_.find(key(p));
    if (it != cells_.end()) return it->second;
    return CellClear;
}

void Buffer::setCell(Cell c, Point p) {
    if (rect.contains(p)) {
        cells_[key(p)] = c;
    }
}

void Buffer::fill(Cell c, Rect r) {
    for (int x = r.min.x; x < r.max.x; ++x)
        for (int y = r.min.y; y < r.max.y; ++y)
            setCell(c, {x, y});
}

void Buffer::setString(const std::u32string& s, Style style, Point p) {
    int x = p.x;
    for (char32_t ch : s) {
        setCell(newCell(ch, style), {x, p.y});
        ++x;  // assume all chars are 1 column wide for now
    }
}

void Buffer::setString(const std::string& s, Style style, Point p) {
    setString(utf8ToU32(s), style, p);
}

} // namespace tui
