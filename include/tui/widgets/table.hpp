#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>
#include <unordered_map>

namespace tui::widgets {

// Table renders a grid of text cells.
// rows[0] is treated as the header row and rendered bold by default.
//
// Example:
//   table.rows = {
//       {"Name",    "Score", "Grade"},   ← header
//       {"Alice",   "98",    "A+"},
//       {"Bob",     "74",    "C"},
//   };
class Table : public Block {
public:
    std::vector<std::vector<std::string>> rows;

    // Optional fixed column widths. If empty, columns are sized equally.
    std::vector<int> columnWidths;

    Style textStyle = Theme.table.text;
    bool  rowSeparator = true;   // draw ─── between rows

    // Per-row style overrides. Key = row index.
    std::unordered_map<int, Style> rowStyles;

    Table();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
