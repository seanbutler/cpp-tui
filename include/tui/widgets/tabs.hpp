#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>

namespace tui::widgets {

// TabPane shows a horizontal row of tab names.
// The active tab is highlighted; use focusLeft/focusRight to move it.
// The widget only draws the tab bar — the caller is responsible for
// showing the corresponding content below it.
//
//  ┌─────────────────────────────────────────────┐
//  │ Overview │ Details │ Logs │ Settings        │
//  └─────────────────────────────────────────────┘
class TabPane : public Block {
public:
    std::vector<std::string> tabNames;
    int   activeTabIndex   = 0;
    Style activeTabStyle   = Theme.tab.active;
    Style inactiveTabStyle = Theme.tab.inactive;

    explicit TabPane(std::vector<std::string> names = {});

    void focusLeft();
    void focusRight();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
