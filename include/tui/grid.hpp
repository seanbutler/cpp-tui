#pragma once
#include "tui/drawable.hpp"
#include <vector>
#include <mutex>

namespace tui {

// ── GridItem ─────────────────────────────────────────────────────────────────
//
// A GridItem is a node in a layout tree: either a leaf holding a widget,
// or a container holding child GridItems.
//
// Usage:
//   Grid grid;
//   grid.set({
//       row(0.5, { col(0.5, &plotWidget), col(0.5, &tableWidget) }),
//       row(0.5, { col(1.0, &gaugeWidget) }),
//   });
//   grid.setRect(0, 0, termWidth, termHeight);
//   render({&grid});

struct GridItem;

// Build a row that shares the given fraction of the parent's height.
GridItem gridRow(double ratio, std::vector<GridItem> children);

// Build a column that shares the given fraction of the parent's width.
GridItem gridCol(double ratio, Drawable* widget);
GridItem gridCol(double ratio, std::vector<GridItem> children);

struct GridItem {
    enum class Type { Row, Col };

    Type   type;
    double ratio = 1.0;
    bool   isLeaf = false;

    Drawable*             widget   = nullptr;
    std::vector<GridItem> children;

    // Computed by Grid::set() — position and size as fractions of the grid rect.
    double xRatio      = 0.0;
    double yRatio      = 0.0;
    double widthRatio  = 0.0;
    double heightRatio = 0.0;
};

// ── Grid ─────────────────────────────────────────────────────────────────────

class Grid : public Drawable {
public:
    Grid();

    // Define the layout tree. Call before setRect().
    void set(std::vector<GridItem> items);

    // Drawable interface
    Rect getRect() const override { return rect_; }
    void setRect(int x1, int y1, int x2, int y2) override;
    void draw(Buffer& buf) override;
    void lock()   override { mutex_.lock(); }
    void unlock() override { mutex_.unlock(); }

private:
    Rect rect_;
    std::mutex mutex_;

    std::vector<GridItem*> leaves_;  // pointers into items_ tree

    // The root item wraps all top-level items as children of a full-height row.
    GridItem root_;

    void computeRatios(GridItem& item, double parentW, double parentH);
    void collectLeaves(GridItem& item);
};

} // namespace tui
