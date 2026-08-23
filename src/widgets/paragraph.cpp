#include "tui/widgets/paragraph.hpp"
#include "tui/style_parser.hpp"
#include "tui/utf8.hpp"

namespace tui::widgets {

Paragraph::Paragraph() {
    textStyle = Theme.paragraph.text;
}

// Split a flat Cell list on '\n', respecting the max width for word-wrap.
static std::vector<std::vector<Cell>> cellLines(
        const std::vector<Cell>& cells, int maxWidth, bool wrap) {

    std::vector<std::vector<Cell>> lines;
    std::vector<Cell> current;

    for (const Cell& c : cells) {
        if (c.rune == U'\n') {
            lines.push_back(current);
            current.clear();
        } else {
            current.push_back(c);
            if (wrap && static_cast<int>(current.size()) >= maxWidth) {
                lines.push_back(current);
                current.clear();
            }
        }
    }
    if (!current.empty()) lines.push_back(current);
    return lines;
}

void Paragraph::draw(Buffer& buf) {
    Block::draw(buf);

    // parseStyles handles both plain text and [markup](fg:red) syntax.
    auto cells = parseStyles(text, textStyle);
    auto lines = cellLines(cells, inner.dx(), wrapText);

    for (int row = 0; row < static_cast<int>(lines.size()); ++row) {
        int y = inner.min.y + row;
        if (y >= inner.max.y) break;

        const auto& line = lines[row];
        for (int col = 0; col < static_cast<int>(line.size()); ++col) {
            int x = inner.min.x + col;
            if (x >= inner.max.x) break;
            buf.setCell(line[col], {x, y});
        }
    }
}

} // namespace tui::widgets
