#pragma once
#include <algorithm>

namespace tui {

struct Point {
    int x = 0;
    int y = 0;

    Point operator+(const Point& o) const { return {x + o.x, y + o.y}; }
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
};

// Rect is an axis-aligned rectangle [min, max).
// max is exclusive (like Go's image.Rectangle).
struct Rect {
    Point min;  // top-left, inclusive
    Point max;  // bottom-right, exclusive

    int dx() const { return max.x - min.x; }
    int dy() const { return max.y - min.y; }

    bool contains(Point p) const {
        return p.x >= min.x && p.x < max.x &&
               p.y >= min.y && p.y < max.y;
    }

    // Returns the intersection of two rectangles (may be empty).
    Rect intersect(const Rect& o) const {
        Rect r;
        r.min.x = std::max(min.x, o.min.x);
        r.min.y = std::max(min.y, o.min.y);
        r.max.x = std::min(max.x, o.max.x);
        r.max.y = std::min(max.y, o.max.y);
        return r;
    }

    bool empty() const { return min.x >= max.x || min.y >= max.y; }
};

// Convenience constructor: makeRect(x1, y1, x2, y2)
inline Rect makeRect(int x1, int y1, int x2, int y2) {
    return Rect{{x1, y1}, {x2, y2}};
}

} // namespace tui
