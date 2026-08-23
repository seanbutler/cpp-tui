#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>

namespace tui::widgets {

// A single sparkline data series.  Not a widget by itself —
// add one or more to a SparklineGroup.
struct Sparkline {
    std::vector<double> data;
    std::string         title;
    Style               titleStyle = Theme.sparkline.title;
    Color               lineColor  = Theme.sparkline.line;
    double              maxVal     = 0.0;  // 0 = auto
};

// SparklineGroup renders one or more Sparklines stacked vertically
// inside a single bordered box.
//
//  ┌ CPU / Memory ───────────────────────────────┐
//  │ CPU                                         │
//  │ ▁▁▂▃▄▄▅▇█▇▆▅▄▃▂▂▁▁▂▃▄▅▆▇█▇▆▅              │
//  │ Memory                                      │
//  │ ▃▃▃▄▄▄▄▄▄▄▅▅▅▅▅▅▅▅▅▅▅▅▆▆▆▆▆▆▆▆            │
//  └─────────────────────────────────────────────┘
class SparklineGroup : public Block {
public:
    std::vector<Sparkline*> sparklines;

    explicit SparklineGroup(std::vector<Sparkline*> sls = {});

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
