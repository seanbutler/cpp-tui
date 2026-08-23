#pragma once
#include "geometry.hpp"
#include "buffer.hpp"

namespace tui {

// Drawable is the interface every widget must implement.
// It mirrors Go's termui Drawable interface.
class Drawable {
public:
    virtual ~Drawable() = default;

    virtual Rect getRect() const = 0;
    virtual void setRect(int x1, int y1, int x2, int y2) = 0;
    virtual void draw(Buffer& buf) = 0;

    // Locking — implementations use a std::mutex.
    virtual void lock()   = 0;
    virtual void unlock() = 0;
};

} // namespace tui
