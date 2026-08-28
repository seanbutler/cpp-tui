#pragma once
#include "style.hpp"
#include <vector>

namespace tui {

// Default colors cycled through for multi-series widgets.
inline const std::vector<Color> StandardColors = {
    ColorRed, ColorGreen, ColorYellow,
    ColorBlue, ColorMagenta, ColorCyan, ColorWhite
};

inline const std::vector<Style> StandardStyles = {
    newStyle(ColorRed),   newStyle(ColorGreen), newStyle(ColorYellow),
    newStyle(ColorBlue),  newStyle(ColorMagenta), newStyle(ColorCyan),
    newStyle(ColorWhite)
};

struct BlockTheme {
    Style title  = newStyle(ColorWhite);
    Style border = newStyle(ColorWhite);
};

struct ParagraphTheme {
    Style text = newStyle(ColorWhite);
};

struct ListTheme {
    Style text     = newStyle(ColorWhite);
    Style selected = newStyle(ColorWhite);
};

struct BarChartTheme {
    std::vector<Color> bars   = StandardColors;
    std::vector<Style> nums   = StandardStyles;
    std::vector<Style> labels = StandardStyles;
};

struct GaugeTheme {
    Color bar        = ColorWhite;
    Style label      = newStyle(ColorWhite);
};

struct PlotTheme {
    std::vector<Color> lines = StandardColors;
    Color              axes  = ColorWhite;
};

struct TableTheme {
    Style text = newStyle(ColorWhite);
};

struct SparklineTheme {
    Style title = newStyle(ColorWhite);
    Color line  = ColorWhite;
};

struct TabTheme {
    Style active   = newStyle(ColorRed);
    Style inactive = newStyle(ColorWhite);
};

struct TreeTheme {
    Style    text      = newStyle(ColorWhite);
    char32_t collapsed = U'+';
    char32_t expanded  = U'−'; // − (minus sign)
};

struct PieChartTheme {
    std::vector<Color> slices = StandardColors;
};

struct StackedBarChartTheme {
    std::vector<Color> bars   = StandardColors;
    std::vector<Style> nums   = StandardStyles;
    std::vector<Style> labels = StandardStyles;
};

// Top-level theme that widgets read their defaults from.
struct RootTheme {
    Style         defaultStyle = newStyle(ColorWhite);
    BlockTheme    block;
    ParagraphTheme paragraph;
    ListTheme     list;
    BarChartTheme barChart;
    GaugeTheme    gauge;
    PlotTheme           plot;
    TableTheme          table;
    SparklineTheme      sparkline;
    TabTheme            tab;
    TreeTheme           tree;
    PieChartTheme       pieChart;
    StackedBarChartTheme stackedBarChart;
};

// Global theme instance — modify before creating widgets to customise defaults.
inline RootTheme Theme;

// Helper: pick item i from a vector, wrapping around if needed.
template<typename T>
const T& selectItem(const std::vector<T>& v, int i) {
    if (v.empty()) {
        static T fallback{};
        return fallback;
    }
    return v[static_cast<size_t>(i) % v.size()];
}

} // namespace tui
