#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>
#include <functional>

namespace tui::widgets {

// StackedBarChart is like BarChart, but each bar is divided into
// coloured segments (one per data[i] sub-value).
//
//   data[i]  = the segments of bar i, bottom-to-top
//   labels[i] = label shown below bar i
//
// Example with 3 bars, each split into 2 segments:
//   data   = { {3,5}, {6,2}, {1,8} }
//   labels = { "A",   "B",   "C"  }
class StackedBarChart : public Block {
public:
    // Each element is one bar; its inner values are the stacked segments.
    std::vector<std::vector<double>> data;
    std::vector<std::string>         labels;

    std::vector<Color> barColors   = Theme.stackedBarChart.bars;
    std::vector<Style> numStyles   = Theme.stackedBarChart.nums;
    std::vector<Style> labelStyles = Theme.stackedBarChart.labels;

    std::function<std::string(double)> numFormatter = [](double v) {
        char buf[16];
        std::snprintf(buf, sizeof(buf), "%.0f", v);
        return std::string(buf);
    };

    int    barWidth = 3;
    int    barGap   = 1;
    double maxVal   = 0.0;  // 0 = auto (max of all column sums)

    StackedBarChart();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
