#include "tui/widgets/barchart.hpp"
#include <algorithm>
#include <cstring>

namespace tui::widgets {

BarChart::BarChart() {
    barColors   = Theme.barChart.bars;
    labelStyles = Theme.barChart.labels;
    numStyles   = Theme.barChart.nums;
}

void BarChart::draw(Buffer& buf) {
    Block::draw(buf);

    if (data.empty()) return;

    // Determine the maximum value for scaling.
    double maxValue = maxVal;
    if (maxValue == 0.0)
        maxValue = *std::max_element(data.begin(), data.end());
    if (maxValue == 0.0) return;

    int barX = inner.min.x;  // current X position for the next bar

    for (int i = 0; i < static_cast<int>(data.size()); ++i) {
        double value = data[i];
        Color  barColor  = selectItem(barColors, i);
        Style  numStyle  = selectItem(numStyles,  i);
        Style  lblStyle  = selectItem(labelStyles, i);

        // Height in terminal rows for this bar (leave 1 row for labels).
        int chartHeight = inner.dy() - 1;
        int barHeight   = static_cast<int>((value / maxValue) * chartHeight);

        // Draw the filled bar (bottom-aligned).
        if (barHeight > 0) {
            int barTop = inner.max.y - 1 - barHeight;  // -1 for label row
            for (int x = barX; x < std::min(barX + barWidth, inner.max.x); ++x) {
                for (int y = barTop; y < inner.max.y - 1; ++y) {
                    buf.setCell(newCell(U' ', newStyle(ColorClear, barColor)), {x, y});
                }
            }
        }

        // Draw the value string inside the bar (just above the label row).
        std::string numStr = numFormatter(value);
        int numX = barX + barWidth / 2;
        if (numX < inner.max.x) {
            buf.setString(numStr,
                          newStyle(numStyle.fg, barColor, numStyle.mod),
                          {numX, inner.max.y - 2});
        }

        // Draw the label below the bar.
        if (i < static_cast<int>(labels.size())) {
            int lblX = barX + barWidth / 2 - static_cast<int>(labels[i].size()) / 2;
            buf.setString(labels[i], lblStyle, {lblX, inner.max.y - 1});
        }

        barX += barWidth + barGap;
        if (barX >= inner.max.x) break;
    }
}

} // namespace tui::widgets
