#include "tui/widgets/stacked_barchart.hpp"
#include <algorithm>
#include <numeric>

namespace tui::widgets {

StackedBarChart::StackedBarChart() {
    barColors   = Theme.stackedBarChart.bars;
    numStyles   = Theme.stackedBarChart.nums;
    labelStyles = Theme.stackedBarChart.labels;
}

void StackedBarChart::draw(Buffer& buf) {
    Block::draw(buf);

    if (data.empty()) return;

    // Determine the max total across all bars.
    double maxValue = maxVal;
    if (maxValue == 0.0) {
        for (const auto& bar : data) {
            double sum = std::accumulate(bar.begin(), bar.end(), 0.0);
            maxValue = std::max(maxValue, sum);
        }
    }
    if (maxValue == 0.0) return;

    int chartHeight = inner.dy() - 1;  // -1 for label row
    int barX        = inner.min.x;

    for (int bi = 0; bi < static_cast<int>(data.size()); ++bi) {
        const auto& segments = data[bi];
        int segmentBase = inner.max.y - 2;  // start drawing from bottom

        for (int si = 0; si < static_cast<int>(segments.size()); ++si) {
            double val    = segments[si];
            int    height = static_cast<int>((val / maxValue) * chartHeight);
            Color  color  = selectItem(barColors, si);
            Style  numSty = selectItem(numStyles,  si);

            // Fill the segment (bottom-up).
            for (int x = barX; x < std::min(barX + barWidth, inner.max.x); ++x) {
                for (int y = segmentBase; y > segmentBase - height; --y) {
                    if (y < inner.min.y) break;
                    buf.setCell(newCell(U' ', newStyle(ColorClear, color)), {x, y});
                }
            }

            // Value label inside the segment.
            int numX = barX + barWidth / 2;
            if (height > 0 && numX < inner.max.x) {
                buf.setString(numFormatter(val),
                              newStyle(numSty.fg, color, numSty.mod),
                              {numX, segmentBase});
            }

            segmentBase -= height;
        }

        // Bar label below.
        if (bi < static_cast<int>(labels.size())) {
            int lblX = barX + barWidth / 2 - static_cast<int>(labels[bi].size()) / 2;
            buf.setString(labels[bi], selectItem(labelStyles, bi),
                          {lblX, inner.max.y - 1});
        }

        barX += barWidth + barGap;
        if (barX >= inner.max.x) break;
    }
}

} // namespace tui::widgets
