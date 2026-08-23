#pragma once
#include "tui/block.hpp"
#include <vector>
#include <string>

namespace tui::widgets {

// List displays a scrollable list of text rows.
// The selected row is highlighted with selectedRowStyle.
class List : public Block {
public:
    std::vector<std::string> rows;
    Style  textStyle        = Theme.list.text;
    Style  selectedRowStyle = Theme.list.text;
    int    selectedRow      = 0;

    List();

    void draw(Buffer& buf) override;

    // Scroll methods — mirroring the Go API.
    void scrollUp()         { scrollAmount(-1); }
    void scrollDown()       { scrollAmount(1); }
    void scrollTop()        { selectedRow = 0; }
    void scrollBottom()     { selectedRow = static_cast<int>(rows.size()) - 1; }
    void scrollPageUp();
    void scrollPageDown();
    void scrollAmount(int delta);

    // Select by absolute visible-row offset from the top of the widget content.
    void selectVisibleRow(int visibleOffset) {
        int abs = topRow_ + visibleOffset;
        int n   = static_cast<int>(rows.size());
        selectedRow = std::clamp(abs, 0, std::max(0, n - 1));
    }

    int topRow() const { return topRow_; }

private:
    int topRow_ = 0;  // index of the first visible row
};

} // namespace tui::widgets
