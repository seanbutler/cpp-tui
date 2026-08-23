# tui-cpp Cheatsheet

One header gets you everything:
```cpp
#include <tui/tui.hpp>
```

---

## Lifecycle

```cpp
tui::init();                        // must be first
auto [w, h] = tui::terminalSize();  // columns, rows
tui::render({&a, &b, &c});          // draw widgets → screen
tui::close();                       // must be last
```

---

## Positioning

Every widget inherits `setRect(x1, y1, x2, y2)` — all values are terminal columns/rows.  
`x2`/`y2` are **exclusive** (like a C++ range).

```cpp
widget.setRect(0, 0, 40, 10);   // 40 wide, 10 tall, top-left corner
```

Common patterns:
```cpp
auto [cols, rows] = tui::terminalSize();
int hw = cols / 2, hh = rows / 2;

topLeft.setRect(0,  0,  hw,   hh);
topRight.setRect(hw, 0,  cols, hh);
botLeft.setRect(0,  hh, hw,   rows);
botRight.setRect(hw, hh, cols, rows);
```

---

## Style

```cpp
tui::Style s = tui::newStyle(fg);
tui::Style s = tui::newStyle(fg, bg);
tui::Style s = tui::newStyle(fg, bg, mod);
```

**Colors** (`tui::Color`):

| Constant | Value |
|---|---|
| `ColorClear` | terminal default |
| `ColorBlack` | 0 |
| `ColorRed` | 1 |
| `ColorGreen` | 2 |
| `ColorYellow` | 3 |
| `ColorBlue` | 4 |
| `ColorMagenta` | 5 |
| `ColorCyan` | 6 |
| `ColorWhite` | 7 |

256-color xterm values (`0`–`255`) are also accepted.

**Modifiers** (`tui::Modifier`, OR-able):

```cpp
tui::ModifierClear      // 0 — no modifier
tui::ModifierBold
tui::ModifierUnderline
tui::ModifierReverse
```

---

## Inline markup (StyleParser)

Syntax: `[text](fg:color, bg:color, mod:modifier)`  
All fields optional. Colors/modifiers use lowercase names.

```cpp
para.text = "[error](fg:red,mod:bold): file not found";
para.text = "[OK](fg:green)  [WARN](fg:yellow)  [ERR](fg:red)";
para.text = "plain [highlighted](fg:white,bg:blue) plain";
```

Colors: `red` `green` `yellow` `blue` `magenta` `cyan` `white` `black` `clear`  
Modifiers: `bold` `underline` `reverse`

---

## Widgets

### Paragraph
```cpp
tui::widgets::Paragraph p;
p.title     = "Title";
p.text      = "Hello\n[world](fg:cyan,mod:bold)";
p.textStyle = tui::newStyle(tui::ColorWhite);
p.wrapText  = true;   // default
p.setRect(0, 0, 40, 10);
```

### List
```cpp
tui::widgets::List l;
l.title             = "Items";
l.rows              = {"alpha", "beta", "gamma"};
l.textStyle         = tui::newStyle(tui::ColorWhite);
l.selectedRowStyle  = tui::newStyle(tui::ColorYellow);
l.selectedRow       = 0;

// Navigation
l.scrollUp();   l.scrollDown();
l.scrollPageUp(); l.scrollPageDown();
l.scrollTop();  l.scrollBottom();
l.scrollAmount(-3);   // relative
```

### BarChart
```cpp
tui::widgets::BarChart b;
b.title    = "Stats";
b.data     = {4.0, 7.5, 2.0, 9.0};
b.labels   = {"A", "B", "C", "D"};
b.barWidth = 3;
b.barGap   = 1;
b.maxVal   = 0.0;   // 0 = auto-scale
b.numFormatter = [](double v) { return std::to_string((int)v); };
```

### Gauge
```cpp
tui::widgets::Gauge g;
g.title    = "Progress";
g.percent  = 42;          // 0–100
g.barColor = tui::ColorGreen;
g.label    = "";          // "" = show "42%", or set custom string
```

### Plot (braille line chart)
```cpp
tui::widgets::Plot p;
p.title      = "CPU";
p.data       = { series1, series2 };   // vector<vector<double>>
p.lineColors = {tui::ColorGreen, tui::ColorYellow};
p.showAxes   = true;
p.maxVal     = 0.0;   // 0 = auto
```

### Table
```cpp
tui::widgets::Table t;
t.title        = "Results";
t.rows         = {
    {"Name",  "Score"},    // row 0 = header (bold by default)
    {"Alice", "98"},
    {"Bob",   "74"},
};
t.columnWidths = {10, 6};  // optional; auto-split if empty
t.rowSeparator = true;
t.rowStyles[0] = tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold);
```

### Sparkline / SparklineGroup
```cpp
tui::widgets::Sparkline sl;
sl.title     = "CPU";
sl.lineColor = tui::ColorGreen;
sl.data      = {10, 40, 30, 80, 60, 90, 20};
sl.maxVal    = 0.0;   // 0 = auto

tui::widgets::SparklineGroup sg({&sl});
sg.title = "Metrics";
```

Multiple sparklines stack vertically:
```cpp
tui::widgets::SparklineGroup sg({&slCPU, &slMem, &slNet});
```

### StackedBarChart
```cpp
tui::widgets::StackedBarChart s;
s.title    = "Server Load";
s.labels   = {"web1", "web2", "web3"};
// data[bar][segment] — segments stack bottom-to-top
s.data     = { {3,2,1}, {5,3,2}, {2,1,4} };
s.barWidth = 5;
s.barGap   = 1;
```

### TabPane
```cpp
tui::widgets::TabPane tabs({"Overview", "Logs", "Settings"});
tabs.activeTabIndex   = 0;
tabs.activeTabStyle   = tui::newStyle(tui::ColorRed);
tabs.inactiveTabStyle = tui::newStyle(tui::ColorWhite);

tabs.focusLeft();    // activeTabIndex--
tabs.focusRight();   // activeTabIndex++
// tabs.activeTabIndex — read to know which content to show
```

### Tree
```cpp
tui::widgets::TreeNode root, child1, child2, leaf;
root.label    = "src";
child1.label  = "widgets";
leaf.label    = "paragraph.cpp";
child1.children = {&leaf};
root.children   = {&child1};
root.expanded   = true;

tui::widgets::Tree tree;
tree.title            = "Files";
tree.selectedRowStyle = tui::newStyle(tui::ColorYellow);
tree.setNodes({&root});

// Navigation
tree.scrollUp();  tree.scrollDown();
tree.toggleExpand();    // expand/collapse selected node
tree.expand();   tree.collapse();
tree.expandAll(); tree.collapseAll();
tree.selectedNode();    // → TreeNode*
```

### PieChart
```cpp
tui::widgets::PieChart pie;
pie.title  = "Market share";
pie.data   = {45, 30, 15, 10};
pie.colors = {tui::ColorBlue, tui::ColorGreen,
              tui::ColorYellow, tui::ColorRed};

// Optional label callback (nullptr = no labels)
pie.labelFormatter = [](int i, double v) {
    char buf[8];
    std::snprintf(buf, sizeof(buf), "%.0f%%", v);
    return std::string(buf);
};
```

---

## Grid layout

```cpp
tui::Grid grid;
grid.set({
    tui::gridRow(0.6, {          // top 60% of height
        tui::gridCol(0.5, &plot),    // left half
        tui::gridCol(0.5, &table),   // right half
    }),
    tui::gridRow(0.4, {          // bottom 40%
        tui::gridCol(1.0, &gauge),   // full width
    }),
});
grid.setRect(0, 0, cols, rows);
tui::render({&grid});
```

Nested columns inside a row:
```cpp
tui::gridCol(0.5, std::vector<tui::GridItem>{
    tui::gridRow(0.5, { tui::gridCol(1.0, &widgetA) }),
    tui::gridRow(0.5, { tui::gridCol(1.0, &widgetB) }),
})
```

---

## Event loop

```cpp
auto events = tui::pollEvents();   // non-blocking background thread

while (true) {
    // Blocking — waits until next event:
    tui::Event e = events->pop();

    // Non-blocking — returns false if queue is empty:
    tui::Event e;
    bool got = events->tryPop(e);

    if (e.type == tui::EventType::Keyboard) { /* e.id */ }
    if (e.type == tui::EventType::Mouse)    { /* e.mouse.x, e.mouse.y */ }
    if (e.type == tui::EventType::Resize)   { /* e.resize.width, e.resize.height */ }

    events->stop();   // shut down the thread
}
```

**Common key ids:**

| Key | id |
|---|---|
| Letters/digits | `"a"` `"Z"` `"3"` |
| Arrow keys | `"<Up>"` `"<Down>"` `"<Left>"` `"<Right>"` |
| Enter | `"<Enter>"` |
| Escape | `"<Escape>"` |
| Tab | `"<Tab>"` |
| Backspace | `"<Backspace>"` |
| Space | `"<Space>"` |
| Ctrl+key | `"<C-c>"` `"<C-d>"` `"<C-z>"` … |
| Function keys | `"<F1>"` … `"<F12>"` |
| Home / End | `"<Home>"` `"<End>"` |
| Page Up/Down | `"<PageUp>"` `"<PageDown>"` |
| Insert / Delete | `"<Insert>"` `"<Delete>"` |

---

## Theme

Modify `tui::Theme` before creating widgets to change global defaults:

```cpp
tui::Theme.block.border = tui::newStyle(tui::ColorCyan);
tui::Theme.block.title  = tui::newStyle(tui::ColorWhite, tui::ColorClear, tui::ModifierBold);
tui::Theme.list.text    = tui::newStyle(tui::ColorGreen);
tui::Theme.gauge.bar    = tui::ColorBlue;
tui::Theme.plot.lines   = {tui::ColorRed, tui::ColorGreen};

tui::init();  // init after setting theme
```

---

## Block base class (all widgets)

```cpp
widget.title        = "My Widget";
widget.titleStyle   = tui::newStyle(tui::ColorWhite, tui::ColorClear, tui::ModifierBold);
widget.border       = true;
widget.borderStyle  = tui::newStyle(tui::ColorWhite);
widget.borderTop    = true;   // each side independently toggleable
widget.borderBottom = true;
widget.borderLeft   = true;
widget.borderRight  = true;
widget.paddingLeft  = 1;      // extra space inside border
widget.paddingRight = 1;
widget.paddingTop   = 0;
widget.paddingBottom = 0;

widget.inner   // Rect — the drawable area inside border+padding (read-only after setRect)
widget.rect    // Rect — the full outer rectangle
```

---

## BrailleCanvas (low-level)

```cpp
tui::BrailleCanvas canvas;

// Braille coordinates: 2× width, 4× height of terminal cells
canvas.setPoint(bx, by, tui::ColorGreen);
canvas.setLine(bx0, by0, bx1, by1, tui::ColorRed);

tui::Buffer buf(myRect);
canvas.draw(buf, myRect);   // clip to rect
canvas.clear();
```

---

## Minimal app template

```cpp
#include <tui/tui.hpp>
#include <chrono>
#include <thread>

int main() {
    tui::init();
    auto [cols, rows] = tui::terminalSize();

    // --- create widgets ---
    tui::widgets::Paragraph p;
    p.text = "Hello!";
    p.setRect(0, 0, cols, rows);

    tui::render({&p});

    auto events = tui::pollEvents();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        tui::Event e;
        while (events->tryPop(e)) {
            if (e.type == tui::EventType::Keyboard) {
                if (e.id == "q" || e.id == "<C-c>") {
                    events->stop();
                    tui::close();
                    return 0;
                }
            }
            if (e.type == tui::EventType::Resize) {
                cols = e.resize.width;
                rows = e.resize.height;
                p.setRect(0, 0, cols, rows);
            }
        }

        // --- update data ---

        tui::render({&p});
    }
}
```
