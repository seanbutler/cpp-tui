#include "tui/widgets/sparkline.hpp"
#include "tui/symbols.hpp"
#include <algorithm>

namespace tui::widgets {

SparklineGroup::SparklineGroup(std::vector<Sparkline*> sls)
    : sparklines(std::move(sls)) {}

void SparklineGroup::draw(Buffer& buf) {
    Block::draw(buf);

    if (sparklines.empty()) return;

    int n          = static_cast<int>(sparklines.size());
    int slotHeight = inner.dy() / n;  // rows allocated to each sparkline

    for (int si = 0; si < n; ++si) {
        Sparkline* sl = sparklines[si];

        // The last sparkline gets any leftover rows.
        int barHeight = (si == n - 1)
            ? inner.dy() - slotHeight * si
            : slotHeight;

        // Bottom y of this sparkline's slot.
        int slotBottom = inner.min.y + slotHeight * (si + 1) - 1;
        if (si == n - 1) slotBottom = inner.max.y - 1;

        // Reserve top row for the title if present.
        int titleRow = -1;
        if (!sl->title.empty()) {
            titleRow = slotBottom - barHeight + 1;
            buf.setString(sl->title, sl->titleStyle, {inner.min.x, titleRow});
            --barHeight;
        }

        if (sl->data.empty() || barHeight <= 0) continue;

        // Find the maximum value for scaling.
        double maxValue = sl->maxVal;
        if (maxValue == 0.0)
            maxValue = *std::max_element(sl->data.begin(), sl->data.end());
        if (maxValue == 0.0) maxValue = 1.0;

        // Draw one column per data point (left → right).
        int width = inner.dx();
        for (int j = 0; j < static_cast<int>(sl->data.size()) && j < width; ++j) {
            double val    = sl->data[j];
            int    height = static_cast<int>((val / maxValue) * barHeight);
            int    x      = inner.min.x + j;

            if (height == 0) {
                // Always show at least a tiny bar so data isn't invisible.
                buf.setCell(newCell(BARS[1], newStyle(sl->lineColor)), {x, slotBottom});
            } else {
                for (int k = 0; k < height; ++k) {
                    int y = slotBottom - k;
                    if (y < inner.min.y) break;
                    buf.setCell(newCell(BARS[8], newStyle(sl->lineColor)), {x, y});
                }
            }
        }
    }
}

} // namespace tui::widgets
