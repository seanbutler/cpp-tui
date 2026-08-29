#include "tui/widgets/tabs.hpp"
#include "tui/symbols.hpp"
#include <algorithm>

namespace tui::widgets {

TabPane::TabPane(std::vector<std::string> names)
    : tabNames(std::move(names)) {}

void TabPane::focusLeft() {
    if (activeTabIndex > 0) --activeTabIndex;
}

void TabPane::focusRight() {
    if (activeTabIndex < static_cast<int>(tabNames.size()) - 1)
        ++activeTabIndex;
}

void TabPane::draw(Buffer& buf) {
    Block::draw(buf);

    int x = inner.min.x;

    for (int i = 0; i < static_cast<int>(tabNames.size()); ++i) {
        if (x >= inner.max.x) break;

        Style style = (i == activeTabIndex) ? activeTabStyle : inactiveTabStyle;

        // Truncate the tab name if it won't fit.
        const auto& name = tabNames[i];
        int available = inner.max.x - x;
        int len = std::min(static_cast<int>(name.size()), available);
        buf.setString(name.substr(0, len), style, {x, inner.min.y});
        x += len;

        x += 2;  // gap between tabs
    }
}

} // namespace tui::widgets
