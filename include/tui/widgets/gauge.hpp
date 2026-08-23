#pragma once
#include "tui/block.hpp"
#include <string>

namespace tui::widgets {

// Gauge shows a horizontal progress bar with a percentage label.
class Gauge : public Block {
public:
    int    percent    = 0;
    Color  barColor   = Theme.gauge.bar;
    std::string label;          // leave empty to auto-show "XX%"
    Style  labelStyle = Theme.gauge.label;

    Gauge();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
