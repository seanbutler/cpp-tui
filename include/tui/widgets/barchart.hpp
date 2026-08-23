#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>
#include <functional>

namespace tui::widgets {

// BarChart renders vertical bars from a list of float values.
// Labels appear below each bar; the value appears inside the bar.
class BarChart : public Block {
public:
    std::vector<double>      data;
    std::vector<std::string> labels;

    std::vector<Color> barColors   = Theme.barChart.bars;
    std::vector<Style> labelStyles = Theme.barChart.labels;
    std::vector<Style> numStyles   = Theme.barChart.nums;

    // Custom formatter for the number inside each bar.
    std::function<std::string(double)> numFormatter = [](double v) {
        // Show integer if whole, otherwise 1 decimal place.
        if (v == static_cast<int>(v))
            return std::to_string(static_cast<int>(v));
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.1f", v);
        return std::string(buf);
    };

    int    barWidth = 3;
    int    barGap   = 1;
    double maxVal   = 0.0;   // 0 = auto (use max of data)

    BarChart();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
