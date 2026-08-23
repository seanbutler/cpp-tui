#include "tui/widgets/plot.hpp"
#include "tui/symbols.hpp"
#include <algorithm>
#include <cstdio>
#include <string>

namespace tui::widgets {

// Space reserved for the Y-axis labels and X-axis labels.
static constexpr int Y_AXIS_WIDTH  = 4;
static constexpr int X_AXIS_HEIGHT = 1;

Plot::Plot() {
    lineColors = Theme.plot.lines;
}

// Find the maximum value across all data series.
static double maxOfData(const std::vector<std::vector<double>>& data) {
    double m = 0.0;
    for (auto& series : data)
        for (double v : series)
            m = std::max(m, v);
    return m;
}

void Plot::drawAxes(Buffer& buf, double maxValue) {
    // Origin cell (corner where axes meet).
    Point origin{inner.min.x + Y_AXIS_WIDTH, inner.max.y - X_AXIS_HEIGHT - 1};
    buf.setCell(newCell(BOTTOM_LEFT, newStyle(ColorWhite)), origin);

    // Horizontal axis (─ ─ ─).
    for (int x = origin.x + 1; x < inner.max.x; ++x)
        buf.setCell(newCell(HORIZONTAL_LINE, newStyle(ColorWhite)), {x, origin.y});

    // Vertical axis.
    for (int y = inner.min.y; y < origin.y; ++y)
        buf.setCell(newCell(VERTICAL_LINE, newStyle(ColorWhite)), {origin.x, y});

    // X-axis label "0" at origin.
    buf.setString("0", newStyle(ColorWhite), {origin.x, inner.max.y - 1});

    // Y-axis labels: evenly spaced values from 0 to maxValue.
    int chartHeight = origin.y - inner.min.y;
    int labelStep   = 2;  // every 2 rows
    double scale    = maxValue / static_cast<double>(chartHeight);

    for (int i = 0; i * labelStep < chartHeight; ++i) {
        double labelVal = i * labelStep * scale;
        char tmp[16];
        std::snprintf(tmp, sizeof(tmp), "%.1f", labelVal);
        int y = origin.y - i * labelStep - 1;
        if (y >= inner.min.y)
            buf.setString(tmp, newStyle(ColorWhite), {inner.min.x, y});
    }
}

void Plot::drawLines(Buffer& buf, Rect drawArea, double maxValue) {
    BrailleCanvas canvas;

    // Braille coordinate space: each terminal cell = 2 wide × 4 tall dots.
    int bWidth  = drawArea.dx() * 2;
    int bHeight = drawArea.dy() * 4;

    for (int si = 0; si < static_cast<int>(data.size()); ++si) {
        const auto& series = data[si];
        Color color = selectItem(lineColors, si);

        if (series.size() < 2) continue;

        int prevBX = 0;
        int prevBY = bHeight - 1 - static_cast<int>((series[0] / maxValue) * (bHeight - 1));

        for (int j = 1; j < static_cast<int>(series.size()); ++j) {
            double clamped = std::min(series[j], maxValue);
            int bx = j * bWidth / static_cast<int>(series.size());
            int by = bHeight - 1 - static_cast<int>((clamped / maxValue) * (bHeight - 1));

            // Translate to absolute braille coords within drawArea.
            canvas.setLine(
                drawArea.min.x * 2 + prevBX, drawArea.min.y * 4 + prevBY,
                drawArea.min.x * 2 + bx,     drawArea.min.y * 4 + by,
                color
            );
            prevBX = bx;
            prevBY = by;
        }
    }

    canvas.draw(buf, drawArea);
}

void Plot::draw(Buffer& buf) {
    Block::draw(buf);

    if (data.empty()) return;

    double maxValue = (maxVal > 0.0) ? maxVal : maxOfData(data);
    if (maxValue == 0.0) maxValue = 1.0;

    if (showAxes)
        drawAxes(buf, maxValue);

    // The actual chart area is inset by the axis labels.
    Rect drawArea = inner;
    if (showAxes) {
        drawArea = makeRect(
            inner.min.x + Y_AXIS_WIDTH + 1,
            inner.min.y,
            inner.max.x,
            inner.max.y - X_AXIS_HEIGHT - 1
        );
    }

    if (drawArea.empty()) return;
    drawLines(buf, drawArea, maxValue);
}

} // namespace tui::widgets
