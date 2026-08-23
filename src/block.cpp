#include "tui/block.hpp"
#include "tui/symbols.hpp"

namespace tui {

void Block::setRect(int x1, int y1, int x2, int y2) {
    rect  = makeRect(x1, y1, x2, y2);
    // Inner rect is rect shrunk by the border (1 cell) plus any padding.
    inner = makeRect(
        rect.min.x + 1 + paddingLeft,
        rect.min.y + 1 + paddingTop,
        rect.max.x - 1 - paddingRight,
        rect.max.y - 1 - paddingBottom
    );
}

void Block::drawBorder(Buffer& buf) {
    Cell vLine  = newCell(VERTICAL_LINE,   borderStyle);
    Cell hLine  = newCell(HORIZONTAL_LINE, borderStyle);

    // Horizontal edges
    if (borderTop)
        buf.fill(hLine, makeRect(rect.min.x, rect.min.y, rect.max.x, rect.min.y + 1));
    if (borderBottom)
        buf.fill(hLine, makeRect(rect.min.x, rect.max.y - 1, rect.max.x, rect.max.y));

    // Vertical edges
    if (borderLeft)
        buf.fill(vLine, makeRect(rect.min.x, rect.min.y, rect.min.x + 1, rect.max.y));
    if (borderRight)
        buf.fill(vLine, makeRect(rect.max.x - 1, rect.min.y, rect.max.x, rect.max.y));

    // Corners
    if (borderTop    && borderLeft)
        buf.setCell(newCell(TOP_LEFT,     borderStyle), rect.min);
    if (borderTop    && borderRight)
        buf.setCell(newCell(TOP_RIGHT,    borderStyle), {rect.max.x - 1, rect.min.y});
    if (borderBottom && borderLeft)
        buf.setCell(newCell(BOTTOM_LEFT,  borderStyle), {rect.min.x,     rect.max.y - 1});
    if (borderBottom && borderRight)
        buf.setCell(newCell(BOTTOM_RIGHT, borderStyle), {rect.max.x - 1, rect.max.y - 1});
}

void Block::draw(Buffer& buf) {
    if (border)
        drawBorder(buf);

    if (!title.empty())
        buf.setString(title, titleStyle, {rect.min.x + 2, rect.min.y});
}

} // namespace tui
