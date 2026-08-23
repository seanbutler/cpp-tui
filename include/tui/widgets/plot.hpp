#pragma once
#include "tui/block.hpp"
#include "tui/canvas.hpp"
#include <vector>

namespace tui::widgets {

// Plot draws one or more data series as a line chart.
// It uses Unicode braille characters for sub-character-cell resolution,
// giving smooth-looking curves even in a text terminal.
class Plot : public Block {
public:
    // Each inner vector is one data series (line).
    std::vector<std::vector<double>> data;

    std::vector<Color> lineColors = Theme.plot.lines;
    bool showAxes = true;
    double maxVal = 0.0;  // 0 = auto (use max of data)

    Plot();

    void draw(Buffer& buf) override;

private:
    void drawAxes(Buffer& buf, double maxValue);
    void drawLines(Buffer& buf, Rect drawArea, double maxValue);
};

} // namespace tui::widgets
