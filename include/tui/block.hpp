#pragma once
#include "drawable.hpp"
#include "style.hpp"
#include "theme.hpp"
#include <string>
#include <mutex>

namespace tui {

// Block is the base class for all widgets.
// It draws a bordered box with an optional title and manages the
// inner rectangle that child widgets draw into.
class Block : public Drawable {
public:
    bool   border      = true;
    Style  borderStyle = Theme.block.border;

    bool borderLeft   = true;
    bool borderRight  = true;
    bool borderTop    = true;
    bool borderBottom = true;

    int paddingLeft   = 0;
    int paddingRight  = 0;
    int paddingTop    = 0;
    int paddingBottom = 0;

    std::string title;
    Style       titleStyle = Theme.block.title;

    Rect rect;   // outer rectangle (full widget area)
    Rect inner;  // inner rectangle (content area inside border+padding)

    // Drawable interface
    Rect getRect() const override { return rect; }
    void setRect(int x1, int y1, int x2, int y2) override;
    void draw(Buffer& buf) override;

    void lock()   override { mutex_.lock(); }
    void unlock() override { mutex_.unlock(); }

protected:
    std::mutex mutex_;

private:
    void drawBorder(Buffer& buf);
};

} // namespace tui
