#pragma once
#include "tui/block.hpp"
#include <vector>
#include <functional>
#include <string>

namespace tui::widgets {

// PieChart renders a filled pie chart using shaded block characters.
// The chart is drawn inside the inner rect; labels are placed at the
// midpoint angle of each slice.
//
// LabelFormatter: receives (sliceIndex, value) and returns a label string.
// Set to nullptr to hide labels.
class PieChart : public Block {
public:
    std::vector<double> data;
    std::vector<Color>  colors        = Theme.pieChart.slices;
    double              angleOffset   = -1.5707963;  // start at top (−π/2)

    std::function<std::string(int, double)> labelFormatter = nullptr;

    PieChart();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
