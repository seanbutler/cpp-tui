#include "tui/widgets/gauge.hpp"
#include <cstdio>

namespace tui::widgets {

Gauge::Gauge() {
    barColor   = Theme.gauge.bar;
    labelStyle = Theme.gauge.label;
}

void Gauge::draw(Buffer& buf) {
    Block::draw(buf);

    // How many columns the filled part covers.
    int barWidth = static_cast<int>((percent / 100.0) * inner.dx());

    // Fill the bar background.
    buf.fill(newCell(U' ', newStyle(ColorClear, barColor)),
             makeRect(inner.min.x, inner.min.y,
                      inner.min.x + barWidth, inner.max.y));

    // Build the label string.
    std::string lbl = label;
    if (lbl.empty()) {
        char tmp[8];
        std::snprintf(tmp, sizeof(tmp), "%d%%", percent);
        lbl = tmp;
    }

    // Centre the label vertically and horizontally.
    int lblX = inner.min.x + (inner.dx() / 2) - static_cast<int>(lbl.size()) / 2;
    int lblY = inner.min.y + (inner.dy() - 1) / 2;

    if (lblY < inner.max.y) {
        for (int i = 0; i < static_cast<int>(lbl.size()); ++i) {
            int x = lblX + i;
            if (x < inner.min.x || x >= inner.max.x) continue;

            // Characters that fall inside the bar get reversed colors.
            Style s = (x < inner.min.x + barWidth)
                ? newStyle(barColor, ColorClear, ModifierReverse)
                : labelStyle;

            buf.setCell(newCell(static_cast<char32_t>(lbl[i]), s), {x, lblY});
        }
    }
}

} // namespace tui::widgets
