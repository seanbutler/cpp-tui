#include "tui/style_parser.hpp"
#include "tui/utf8.hpp"
#include <unordered_map>
#include <sstream>

namespace tui {

static const std::unordered_map<std::string, Color> COLOR_MAP = {
    {"red",     ColorRed},
    {"green",   ColorGreen},
    {"yellow",  ColorYellow},
    {"blue",    ColorBlue},
    {"magenta", ColorMagenta},
    {"cyan",    ColorCyan},
    {"white",   ColorWhite},
    {"black",   ColorBlack},
    {"clear",   ColorClear},
};

static const std::unordered_map<std::string, Modifier> MODIFIER_MAP = {
    {"bold",      ModifierBold},
    {"underline", ModifierUnderline},
    {"reverse",   ModifierReverse},
    {"dim",       ModifierDim},
};

// Parse "fg:red,mod:bold,bg:blue" into a Style, falling back to defaultStyle.
static Style parseStyleSpec(const std::string& spec, Style defaultStyle) {
    Style s = defaultStyle;
    std::istringstream stream(spec);
    std::string item;
    while (std::getline(stream, item, ',')) {
        auto colon = item.find(':');
        if (colon == std::string::npos) continue;
        std::string key = item.substr(0, colon);
        std::string val = item.substr(colon + 1);

        if (key == "fg") {
            auto it = COLOR_MAP.find(val);
            if (it != COLOR_MAP.end()) s.fg = it->second;
        } else if (key == "bg") {
            auto it = COLOR_MAP.find(val);
            if (it != COLOR_MAP.end()) s.bg = it->second;
        } else if (key == "mod") {
            auto it = MODIFIER_MAP.find(val);
            if (it != MODIFIER_MAP.end()) s.mod = it->second;
        }
    }
    return s;
}

// Append a span of UTF-32 characters to cells using the given style.
static void appendCells(std::vector<Cell>& cells,
                        const std::u32string& text, Style style) {
    for (char32_t ch : text)
        cells.push_back(newCell(ch, style));
}

std::vector<Cell> parseStyles(const std::string& s, Style defaultStyle) {
    std::vector<Cell> cells;
    std::u32string text = utf8ToU32(s);

    // State machine — mirrors the Go implementation.
    enum class State { Default, InText, InStyle };

    State       state      = State::Default;
    std::u32string styledText;
    std::u32string styleSpec;
    int         bracketDepth = 0;

    auto rollback = [&]() {
        // Couldn't complete the markup — emit accumulated chars as plain text.
        appendCells(cells, styledText, defaultStyle);
        appendCells(cells, styleSpec, defaultStyle);
        styledText.clear();
        styleSpec.clear();
        state = State::Default;
        bracketDepth = 0;
    };

    for (size_t i = 0; i < text.size(); ++i) {
        char32_t ch = text[i];

        switch (state) {
        case State::Default:
            if (ch == U'[') {
                state = State::InText;
                bracketDepth = 1;
                styledText.clear();
            } else {
                cells.push_back(newCell(ch, defaultStyle));
            }
            break;

        case State::InText:
            if (ch == U'[') {
                ++bracketDepth;
                styledText += ch;
            } else if (ch == U']') {
                --bracketDepth;
                if (bracketDepth == 0) {
                    // Check next char is '('.
                    if (i + 1 < text.size() && text[i + 1] == U'(') {
                        state = State::InStyle;
                        styleSpec.clear();
                        ++i; // skip '('
                    } else {
                        // No style follows — rollback.
                        rollback();
                    }
                } else {
                    styledText += ch;
                }
            } else if (i + 1 == text.size()) {
                // End of string inside text — rollback.
                styledText += ch;
                rollback();
            } else {
                styledText += ch;
            }
            break;

        case State::InStyle:
            if (ch == U')') {
                // Complete markup — apply the style.
                std::string specUtf8;
                for (char32_t c : styleSpec)
                    specUtf8 += static_cast<char>(c); // spec is always ASCII
                Style style = parseStyleSpec(specUtf8, defaultStyle);
                appendCells(cells, styledText, style);
                styledText.clear();
                styleSpec.clear();
                state = State::Default;
            } else if (i + 1 == text.size()) {
                // End of string inside style spec — rollback.
                styleSpec += ch;
                rollback();
            } else {
                styleSpec += ch;
            }
            break;
        }
    }

    return cells;
}

} // namespace tui
