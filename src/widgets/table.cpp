#include "tui/widgets/table.hpp"
#include "tui/symbols.hpp"
#include <algorithm>

namespace tui::widgets {

Table::Table() {
    textStyle = Theme.table.text;
    rowStyles[0] = newStyle(ColorWhite, ColorClear, ModifierBold);
}

// ── Scroll / selection ────────────────────────────────────────────────────────

void Table::scrollAmount(int delta) {
    int n = static_cast<int>(rows.size());
    if (n <= 1) return;  // header only

    // Initialise selectedRow if nothing is selected yet.
    if (selectedRow < 1) selectedRow = 1;

    selectedRow = std::clamp(selectedRow + delta, 1, n - 1);

    // Compute how many data rows fit in the inner area (header takes 1 rowHeight).
    int headerLines = rowHeight();
    int visibleData = (inner.dy() - headerLines) / rowHeight();

    // Scroll topRow_ so selected row stays on screen.
    if (selectedRow >= topRow_ + visibleData)
        topRow_ = selectedRow - visibleData + 1;
    else if (selectedRow < topRow_)
        topRow_ = selectedRow;

    topRow_ = std::max(topRow_, 1);
}

void Table::scrollBottom() {
    int n = static_cast<int>(rows.size());
    if (n <= 1) return;
    selectedRow = n - 1;
    int headerLines  = rowHeight();
    int visibleData  = (inner.dy() - headerLines) / rowHeight();
    topRow_ = std::max(1, selectedRow - visibleData + 1);
}

void Table::selectVisibleRow(int visibleOffset) {
    // The header is always the first thing drawn (takes rowHeight() lines).
    // visibleOffset is counted from inner.min.y.
    int rh = rowHeight();
    if (visibleOffset < rh) return;  // clicked on the header — no selection

    int clickedDataSlot = (visibleOffset - rh) / rh;  // 0-based data row slot
    int absRow = topRow_ + clickedDataSlot;
    int n = static_cast<int>(rows.size());
    if (absRow >= 1 && absRow < n)
        selectedRow = absRow;
}

// ── Drawing ───────────────────────────────────────────────────────────────────

void Table::draw(Buffer& buf) {
    Block::draw(buf);

    if (rows.empty()) return;

    // ── Compute column widths ─────────────────────────────────────────────────
    std::vector<int> colWidths = columnWidths;
    if (colWidths.empty()) {
        int colCount = static_cast<int>(rows[0].size());
        int colWidth = inner.dx() / std::max(colCount, 1);
        colWidths.assign(colCount, colWidth);
    }

    // ── Helper: draw one row at terminal row y ────────────────────────────────
    auto drawRow = [&](int ri, int y) {
        if (y >= inner.max.y) return;
        const auto& row = rows[ri];

        // Determine style: selected > explicit override > default.
        Style rowStyle = textStyle;
        if (ri == selectedRow) {
            rowStyle = selectedRowStyle;
        } else {
            auto it = rowStyles.find(ri);
            if (it != rowStyles.end()) rowStyle = it->second;
        }

        // Fill the entire row background so the selection highlight spans full width.
        if (ri == selectedRow)
            buf.fill(newCell(U' ', rowStyle), makeRect(inner.min.x, y, inner.max.x, y + 1));

        int x = inner.min.x;
        for (int ci = 0; ci < static_cast<int>(row.size()); ++ci) {
            int colW  = (ci < static_cast<int>(colWidths.size())) ? colWidths[ci] : 8;
            int limit = std::min(colW, inner.max.x - x);

            const auto& text = row[ci];
            for (int k = 0; k < limit && k < static_cast<int>(text.size()); ++k) {
                bool trunc = (k == limit - 1 && static_cast<int>(text.size()) > limit);
                buf.setCell(newCell(trunc ? ELLIPSES : static_cast<char32_t>(text[k]),
                                    rowStyle), {x + k, y});
            }

            x += colW;

            // Column separator.
            if (ci < static_cast<int>(row.size()) - 1 && x < inner.max.x) {
                buf.setCell(newCell(VERTICAL_LINE, borderStyle), {x, y});
                ++x;
            }
        }
    };

    auto drawSeparator = [&](int y) {
        if (y < inner.max.y)
            buf.fill(newCell(HORIZONTAL_LINE, borderStyle),
                     makeRect(inner.min.x, y, inner.max.x, y + 1));
    };

    int rh = rowHeight();
    int y  = inner.min.y;

    // Always draw the header (row 0) first, pinned to the top.
    drawRow(0, y);
    y += 1;
    if (rowSeparator) { drawSeparator(y); y += 1; }

    // Draw data rows starting from topRow_.
    for (int ri = topRow_; ri < static_cast<int>(rows.size()) && y < inner.max.y; ++ri) {
        drawRow(ri, y);
        y += 1;
        if (rowSeparator && ri < static_cast<int>(rows.size()) - 1) {
            drawSeparator(y);
            y += 1;
        }
    }

    // Scroll arrows.
    if (topRow_ > 1)
        buf.setCell(newCell(UP_ARROW,   newStyle(ColorWhite)), {inner.max.x - 1, inner.min.y + rh});
    int visibleData = (inner.dy() - rh) / rh;
    if (topRow_ + visibleData < static_cast<int>(rows.size()))
        buf.setCell(newCell(DOWN_ARROW, newStyle(ColorWhite)), {inner.max.x - 1, inner.max.y - 1});
}

} // namespace tui::widgets
