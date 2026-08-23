#include "tui/grid.hpp"

namespace tui {

// ── Builder functions ────────────────────────────────────────────────────────

GridItem gridRow(double ratio, std::vector<GridItem> children) {
    GridItem item;
    item.type     = GridItem::Type::Row;
    item.ratio    = ratio;
    item.isLeaf   = false;
    item.children = std::move(children);
    return item;
}

GridItem gridCol(double ratio, Drawable* widget) {
    GridItem item;
    item.type   = GridItem::Type::Col;
    item.ratio  = ratio;
    item.isLeaf = true;
    item.widget = widget;
    return item;
}

GridItem gridCol(double ratio, std::vector<GridItem> children) {
    GridItem item;
    item.type     = GridItem::Type::Col;
    item.ratio    = ratio;
    item.isLeaf   = false;
    item.children = std::move(children);
    return item;
}

// ── Grid ─────────────────────────────────────────────────────────────────────

Grid::Grid() {
    root_.type   = GridItem::Type::Row;
    root_.ratio  = 1.0;
    root_.isLeaf = false;
}

void Grid::set(std::vector<GridItem> items) {
    root_.children = std::move(items);
    root_.xRatio      = 0.0;
    root_.yRatio      = 0.0;
    root_.widthRatio  = 1.0;
    root_.heightRatio = 1.0;

    leaves_.clear();
    computeRatios(root_, 1.0, 1.0);
    collectLeaves(root_);
}

// Recursively compute each item's widthRatio and heightRatio.
// The rules (mirroring the Go implementation):
//   - A Col takes `ratio` of the parent width, full parent height.
//   - A Row takes full parent width, `ratio` of the parent height.
void Grid::computeRatios(GridItem& item, double parentW, double parentH) {
    double w = (item.type == GridItem::Type::Col) ? item.ratio : 1.0;
    double h = (item.type == GridItem::Type::Row) ? item.ratio : 1.0;
    item.widthRatio  = parentW * w;
    item.heightRatio = parentH * h;

    if (item.isLeaf) return;

    double xOff = item.xRatio;
    double yOff = item.yRatio;

    for (auto& child : item.children) {
        child.xRatio = xOff;
        child.yRatio = yOff;

        computeRatios(child, item.widthRatio, item.heightRatio);

        if (child.type == GridItem::Type::Col)
            xOff += child.widthRatio;
        else
            yOff += child.heightRatio;
    }
}

void Grid::collectLeaves(GridItem& item) {
    if (item.isLeaf) {
        leaves_.push_back(&item);
        return;
    }
    for (auto& child : item.children)
        collectLeaves(child);
}

void Grid::setRect(int x1, int y1, int x2, int y2) {
    rect_ = makeRect(x1, y1, x2, y2);
}

void Grid::draw(Buffer& buf) {
    double w = static_cast<double>(rect_.dx());
    double h = static_cast<double>(rect_.dy());

    for (GridItem* leaf : leaves_) {
        if (!leaf->widget) continue;

        int x = rect_.min.x + static_cast<int>(w * leaf->xRatio);
        int y = rect_.min.y + static_cast<int>(h * leaf->yRatio);
        int x2 = x + static_cast<int>(w * leaf->widthRatio);
        int y2 = y + static_cast<int>(h * leaf->heightRatio);

        leaf->widget->setRect(x, y, x2, y2);
        leaf->widget->lock();
        leaf->widget->draw(buf);
        leaf->widget->unlock();
    }
}

} // namespace tui
