#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>
#include <unordered_map>
#include <algorithm>

namespace tui::widgets {

// Table renders a grid of text cells with optional row selection and scrolling.
// rows[0] is the header — it is skipped by selection and scroll.
class Table : public Block {
public:
    std::vector<std::vector<std::string>> rows;

    // Optional fixed column widths. If empty, columns are sized equally.
    std::vector<int> columnWidths;

    Style textStyle        = Theme.table.text;
    Style selectedRowStyle = newStyle(ColorBlack, ColorCyan);
    bool  rowSeparator     = true;

    // -1 = no selection.  Refers to the absolute index in `rows`.
    int selectedRow = -1;

    // Per-row style overrides (e.g. header styling). Applied after selection.
    std::unordered_map<int, Style> rowStyles;

    Table();

    void draw(Buffer& buf) override;

    // Navigation — skips the header row (index 0).
    void scrollUp()   { scrollAmount(-1); }
    void scrollDown() { scrollAmount(1); }
    void scrollTop()  { selectedRow = (rows.size() > 1) ? 1 : -1; topRow_ = 1; }
    void scrollBottom();
    void scrollAmount(int delta);

    // Select by click offset from the top of the inner content area.
    // Accounts for row-separator lines automatically.
    void selectVisibleRow(int visibleOffset);

    int topRow() const { return topRow_; }

private:
    int topRow_ = 1;  // first visible row (1 = skip header which is always shown)

    // Number of terminal lines one data row occupies.
    int rowHeight() const { return rowSeparator ? 2 : 1; }
};

} // namespace tui::widgets
