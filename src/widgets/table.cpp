#include "tui/widgets/table.hpp"
#include "tui/symbols.hpp"
#include <algorithm>

namespace tui::widgets {

Table::Table() {
    textStyle = Theme.table.text;
    // Give the header row a bold style by default.
    rowStyles[0] = newStyle(ColorWhite, ColorClear, ModifierBold);
}

void Table::draw(Buffer& buf) {
    Block::draw(buf);

    if (rows.empty()) return;

    // ── Compute column widths ────────────────────────────────────────────────
    std::vector<int> colWidths = columnWidths;
    if (colWidths.empty()) {
        int colCount  = static_cast<int>(rows[0].size());
        int colWidth  = inner.dx() / std::max(colCount, 1);
        colWidths.assign(colCount, colWidth);
    }

    // ── Draw rows ────────────────────────────────────────────────────────────
    int y = inner.min.y;

    for (int ri = 0; ri < static_cast<int>(rows.size()) && y < inner.max.y; ++ri) {
        const auto& row = rows[ri];

        // Use row-specific style if one is set, otherwise fall back to default.
        Style rowStyle = textStyle;
        auto  it       = rowStyles.find(ri);
        if (it != rowStyles.end()) rowStyle = it->second;

        // Draw each cell in this row.
        int x = inner.min.x;
        for (int ci = 0; ci < static_cast<int>(row.size()); ++ci) {
            int colW = (ci < static_cast<int>(colWidths.size())) ? colWidths[ci] : 8;

            // Draw the cell text, truncating with … if it's too wide.
            const auto& text = row[ci];
            int limit = std::min(colW, inner.max.x - x);
            for (int k = 0; k < limit && k < static_cast<int>(text.size()); ++k) {
                bool truncated = (k == limit - 1 &&
                                  static_cast<int>(text.size()) > limit);
                char32_t ch = truncated ? ELLIPSES : static_cast<char32_t>(text[k]);
                buf.setCell(newCell(ch, rowStyle), {x + k, y});
            }

            x += colW;

            // Draw a vertical separator after each column (except the last).
            if (ci < static_cast<int>(row.size()) - 1 && x < inner.max.x) {
                buf.setCell(newCell(VERTICAL_LINE, borderStyle), {x, y});
                ++x;
            }
        }

        ++y;

        // Draw a horizontal separator between rows (not after the last row).
        if (rowSeparator && ri < static_cast<int>(rows.size()) - 1 && y < inner.max.y) {
            buf.fill(newCell(HORIZONTAL_LINE, borderStyle),
                     makeRect(inner.min.x, y, inner.max.x, y + 1));
            ++y;
        }
    }
}

} // namespace tui::widgets
