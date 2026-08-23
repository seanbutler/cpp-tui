#include "tui/widgets/piechart.hpp"
#include "tui/symbols.hpp"
#include <cmath>
#include <numeric>
#include <algorithm>

namespace tui::widgets {

// Terminal cells are roughly twice as tall as wide, so we stretch x by 2
// to make circles look round instead of squashed.
static constexpr double X_STRETCH = 2.0;
static constexpr double TWO_PI    = 2.0 * M_PI;

PieChart::PieChart() {
    colors = Theme.pieChart.slices;
}

void PieChart::draw(Buffer& buf) {
    Block::draw(buf);

    if (data.empty()) return;

    double total = std::accumulate(data.begin(), data.end(), 0.0);
    if (total == 0.0) return;

    // Center of the inner rect.
    double cx = inner.min.x + inner.dx() / 2.0;
    double cy = inner.min.y + inner.dy() / 2.0;

    // Radius: constrained so the circle fits even with the x-stretch.
    double radius = std::min(inner.dx() / 2.0 / X_STRETCH,
                             inner.dy() / 2.0);

    // For every cell in the inner rect, determine which slice it belongs to.
    for (int y = inner.min.y; y < inner.max.y; ++y) {
        for (int x = inner.min.x; x < inner.max.x; ++x) {
            // Normalised coordinates relative to center (un-stretch x).
            double dx = (x - cx) / X_STRETCH;
            double dy = y - cy;
            double dist = std::sqrt(dx * dx + dy * dy);

            if (dist > radius) continue;

            // Angle from center, normalised to [0, 2π) relative to angleOffset.
            double angle = std::atan2(dy, dx) - angleOffset;
            angle = std::fmod(angle + TWO_PI * 4, TWO_PI);  // force into [0, 2π)

            // Find which slice this angle falls in.
            double sweep = 0.0;
            for (int si = 0; si < static_cast<int>(data.size()); ++si) {
                sweep += data[si] / total * TWO_PI;
                if (angle <= sweep) {
                    Color c = selectItem(colors, si);
                    buf.setCell(newCell(SHADED_BLOCKS[1], newStyle(c)), {x, y});
                    break;
                }
            }
        }
    }

    // Draw labels at the midpoint of each slice.
    if (!labelFormatter) return;

    double angle = angleOffset;
    for (int si = 0; si < static_cast<int>(data.size()); ++si) {
        double sliceAngle = data[si] / total * TWO_PI;
        double mid        = angle + sliceAngle / 2.0;

        // Place label at half the radius.
        int lx = static_cast<int>(std::round(cx + X_STRETCH * (radius / 2.0) * std::cos(mid)));
        int ly = static_cast<int>(std::round(cy + (radius / 2.0) * std::sin(mid)));

        std::string lbl = labelFormatter(si, data[si]);
        Color c = selectItem(colors, si);

        // Centre the label string on (lx, ly).
        int startX = lx - static_cast<int>(lbl.size()) / 2;
        for (int k = 0; k < static_cast<int>(lbl.size()); ++k) {
            Point p{startX + k, ly};
            if (inner.contains(p))
                buf.setCell(newCell(static_cast<char32_t>(lbl[k]), newStyle(c)), p);
        }

        angle += sliceAngle;
    }
}

} // namespace tui::widgets
