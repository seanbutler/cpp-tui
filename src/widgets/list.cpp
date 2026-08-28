#include "tui/widgets/list.hpp"
#include "tui/symbols.hpp"
#include <algorithm>

namespace tui::widgets {

List::List() {
    textStyle        = Theme.list.text;
    selectedRowStyle = Theme.list.selected;
}

void List::scrollAmount(int delta) {
    int n = static_cast<int>(rows.size());
    selectedRow = std::clamp(selectedRow + delta, 0, std::max(0, n - 1));
}

void List::scrollPageUp() {
    if (selectedRow > topRow_)
        selectedRow = topRow_;
    else
        scrollAmount(-inner.dy());
}

void List::scrollPageDown() {
    scrollAmount(inner.dy());
}

void List::draw(Buffer& buf) {
    Block::draw(buf);

    int visibleHeight = inner.dy();
    int rowCount      = static_cast<int>(rows.size());

    // Adjust topRow so the selected row is always visible.
    if (selectedRow >= topRow_ + visibleHeight)
        topRow_ = selectedRow - visibleHeight + 1;
    else if (selectedRow < topRow_)
        topRow_ = selectedRow;

    // Draw each visible row.
    int y = inner.min.y;
    for (int row = topRow_; row < rowCount && y < inner.max.y; ++row, ++y) {
        Style style = (row == selectedRow) ? selectedRowStyle : textStyle;

        int x = inner.min.x;
        for (char ch : rows[row]) {
            if (x >= inner.max.x) {
                // Row is too wide — show ellipsis at the last column.
                buf.setCell(newCell(ELLIPSES, style), {inner.max.x - 1, y});
                break;
            }
            buf.setCell(newCell(static_cast<char32_t>(ch), style), {x, y});
            ++x;
        }
    }

    // Scroll-indicator arrows.
    if (topRow_ > 0)
        buf.setCell(newCell(UP_ARROW, newStyle(ColorWhite)), {inner.max.x - 1, inner.min.y});

    if (rowCount > topRow_ + visibleHeight)
        buf.setCell(newCell(DOWN_ARROW, newStyle(ColorWhite)), {inner.max.x - 1, inner.max.y - 1});
}

} // namespace tui::widgets
