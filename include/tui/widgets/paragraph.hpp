#pragma once
#include "tui/block.hpp"
#include <string>

namespace tui::widgets {

// Paragraph displays a text string inside a bordered box.
// Supports optional word-wrap within the inner rectangle.
class Paragraph : public Block {
public:
    std::string text;
    Style       textStyle = Theme.paragraph.text;
    bool        wrapText  = true;

    Paragraph();

    void draw(Buffer& buf) override;
};

} // namespace tui::widgets
