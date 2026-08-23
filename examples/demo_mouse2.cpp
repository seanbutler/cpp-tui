#include <tui/tui.hpp>
#include <cmath>
#include <chrono>
#include <thread>
#include <vector>
#include <string>

// ── View mode ─────────────────────────────────────────────────────────────────

enum class Mode { Tiled, Expanded };

// ── Panel descriptor ──────────────────────────────────────────────────────────

struct Panel {
    std::string  name;
    tui::Block*  widget;
};

// ── Button drawing / hit-testing ──────────────────────────────────────────────
// Buttons are 3 characters wide: "[X]"
// They are drawn directly into the shared frame buffer, sitting on top of
// whatever border character was already there.

static const tui::Style BTN_NORMAL  = tui::newStyle(tui::ColorYellow, tui::ColorClear, tui::ModifierBold);
static const tui::Style BTN_FOCUSED = tui::newStyle(tui::ColorCyan,   tui::ColorClear, tui::ModifierBold);

static void drawBtn(tui::Buffer& buf, tui::Point p, char32_t icon, tui::Style s) {
    buf.setCell(tui::newCell(U'[',  s), {p.x,     p.y});
    buf.setCell(tui::newCell(icon,  s), {p.x + 1, p.y});
    buf.setCell(tui::newCell(U']',  s), {p.x + 2, p.y});
}

static bool hitBtn(int mx, int my, tui::Point p) {
    return my == p.y && mx >= p.x && mx <= p.x + 2;
}

// Each panel's expand button sits on its top border, 4 chars from the right.
static tui::Point expandPos(const Panel& p) {
    return {p.widget->rect.max.x - 4, p.widget->rect.min.y};
}

// The collapse button lives at the far right of row 0 (the tab bar top border).
static tui::Point collapsePos(int cols) {
    return {cols - 4, 0};
}

// ── Sine helper ───────────────────────────────────────────────────────────────

static std::vector<double> sineWave(int n, double phase, double amp = 1.0) {
    std::vector<double> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = amp * (std::sin(phase + i * 0.15) * 0.5 + 0.5);
    return v;
}

// ── Layout helpers ────────────────────────────────────────────────────────────

static constexpr int TAB_H = 3;  // rows taken by the tab bar in expanded mode

static void doTiledLayout(std::vector<Panel>& panels, int cols, int rows) {
    int hw = cols / 2, hh = rows / 2;
    panels[0].widget->setRect(0,  0,  hw,   hh);
    panels[1].widget->setRect(hw, 0,  cols, hh);
    panels[2].widget->setRect(0,  hh, hw,   rows);
    panels[3].widget->setRect(hw, hh, cols, rows);
}

static void doExpandedLayout(Panel& p, int cols, int rows) {
    p.widget->setRect(0, TAB_H, cols, rows);
}

// ── Focus colour ──────────────────────────────────────────────────────────────

static void applyFocus(std::vector<Panel>& panels, int focused) {
    for (int i = 0; i < 4; ++i) {
        bool on = (i == focused);
        panels[i].widget->borderStyle =
            on ? tui::newStyle(tui::ColorCyan)  : tui::newStyle(tui::ColorWhite);
        panels[i].widget->titleStyle  =
            on ? tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold)
               : tui::newStyle(tui::ColorWhite);
    }
}

// ── Frame renderer ────────────────────────────────────────────────────────────

static void drawFrame(
        tui::Buffer&          frame,
        std::vector<Panel>&   panels,
        tui::widgets::TabPane& tabs,
        Mode                  mode,
        int                   active,
        int                   cols) {

    if (mode == Mode::Tiled) {
        for (int i = 0; i < 4; ++i) {
            Panel& p = panels[i];
            p.widget->lock();
            p.widget->draw(frame);
            p.widget->unlock();
            // Expand button on the top border.
            bool focused = (i == active);
            drawBtn(frame, expandPos(p), U'+', focused ? BTN_FOCUSED : BTN_NORMAL);
        }
        // One-line hint at the very bottom.
        tui::Style hint = tui::newStyle(tui::ColorWhite);
        frame.setString(" [+] expand  |  click to focus  |  Tab cycle  |  q quit",
                        hint, {0, frame.rect.max.y - 1});
    } else {
        // Tab bar.
        tabs.lock();
        tabs.draw(frame);
        tabs.unlock();
        // Collapse button on the tab bar's top border.
        drawBtn(frame, collapsePos(cols), U'-', BTN_NORMAL);

        // Active panel content.
        Panel& p = panels[active];
        p.widget->lock();
        p.widget->draw(frame);
        p.widget->unlock();
        // The expand button is reused as a visual indicator (greyed out).
        drawBtn(frame, expandPos(p), U'-', BTN_FOCUSED);
    }

    tui::flushBuffer(frame);
    tui::screenRefresh();
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
    tui::init();
    auto [cols, rows] = tui::terminalSize();

    // ── List ─────────────────────────────────────────────────────────────────
    tui::widgets::List list;
    list.title = "List";
    list.rows  = {
        "Aardvark","Baboon","Capybara","Dingo","Elephant",
        "Flamingo","Gecko","Hamster","Iguana","Jellyfish",
        "Kangaroo","Lemur","Manatee","Narwhal","Octopus",
        "Pangolin","Quokka","Raccoon","Salamander","Tapir",
    };
    list.selectedRowStyle = tui::newStyle(tui::ColorBlack, tui::ColorCyan);

    // ── Plot ─────────────────────────────────────────────────────────────────
    tui::widgets::Plot plot;
    plot.title      = "Plot";
    plot.lineColors = {tui::ColorGreen, tui::ColorYellow, tui::ColorRed};

    // ── Tree ─────────────────────────────────────────────────────────────────
    std::vector<tui::widgets::TreeNode> tn(12);
    tn[0].label="Documents"; tn[1].label="report.pdf"; tn[2].label="notes.txt";
    tn[3].label="Projects";  tn[4].label="tui-cpp";
    tn[5].label="src";       tn[6].label="buffer.cpp"; tn[7].label="terminal.cpp";
    tn[8].label="include";   tn[9].label="tui.hpp";
    tn[10].label="Downloads";tn[11].label="photo.jpg";

    tn[5].children={&tn[6],&tn[7]}; tn[8].children={&tn[9]};
    tn[4].children={&tn[5],&tn[8]}; tn[3].children={&tn[4]};
    tn[0].children={&tn[1],&tn[2]}; tn[10].children={&tn[11]};
    tn[0].expanded=tn[3].expanded=tn[4].expanded=tn[10].expanded=true;

    tui::widgets::Tree tree;
    tree.title            = "Tree";
    tree.selectedRowStyle = tui::newStyle(tui::ColorBlack, tui::ColorCyan);
    tree.setNodes({&tn[0], &tn[3], &tn[10]});

    // ── Table ─────────────────────────────────────────────────────────────────
    tui::widgets::Table table;
    table.title = "Table";
    table.rows  = {
        {"Language","Paradigm",   "Year"},
        {"C++",     "Multi",      "1983"},
        {"Rust",    "Systems",    "2010"},
        {"Go",      "Concurrent", "2009"},
        {"Python",  "Dynamic",    "1991"},
        {"Haskell", "Functional", "1990"},
        {"Zig",     "Systems",    "2016"},
        {"Nim",     "Multi",      "2008"},
        {"Julia",   "Scientific", "2012"},
        {"Swift",   "Multi",      "2014"},
    };
    table.rowStyles[0] = tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold);

    // ── Panels + tab bar ──────────────────────────────────────────────────────
    std::vector<Panel> panels = {
        {"List",  &list},
        {"Plot",  &plot},
        {"Tree",  &tree},
        {"Table", &table},
    };

    tui::widgets::TabPane tabs({"List", "Plot", "Tree", "Table"});
    tabs.setRect(0, 0, cols, TAB_H);
    tabs.activeTabStyle   = tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold);
    tabs.inactiveTabStyle = tui::newStyle(tui::ColorWhite);

    // ── State + transition helpers ────────────────────────────────────────────
    Mode mode        = Mode::Tiled;
    int  active      = 0;

    auto switchTo = [&](int idx) {
        active = idx;
        tabs.activeTabIndex = idx;
        applyFocus(panels, active);
        if (mode == Mode::Expanded)
            doExpandedLayout(panels[active], cols, rows);
    };

    auto expand = [&](int idx) {
        mode = Mode::Expanded;
        switchTo(idx);
    };

    auto collapse = [&]() {
        mode = Mode::Tiled;
        doTiledLayout(panels, cols, rows);
        applyFocus(panels, active);
    };

    doTiledLayout(panels, cols, rows);
    applyFocus(panels, active);

    // ── Event loop ────────────────────────────────────────────────────────────
    auto events = tui::pollEvents();
    double phase = 0.0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        phase += 0.12;
        plot.data = { sineWave(80, phase), sineWave(80, phase + 1.0), sineWave(80, phase + 2.0) };

        tui::Event e;
        while (events->tryPop(e)) {

            // ── Keyboard ─────────────────────────────────────────────────────
            if (e.type == tui::EventType::Keyboard) {
                const auto& id = e.id;

                if (id == "q" || id == "<C-c>") {
                    events->stop();
                    tui::close();
                    return 0;
                }
                if (id == "<Tab>")
                    switchTo((active + 1) % 4);

                if (id == "f" || id == "<Enter>") {
                    if (mode == Mode::Tiled) expand(active);
                    else                     collapse();
                }
                if (id == "<Escape>" && mode == Mode::Expanded)
                    collapse();

                if (id == "<Up>") {
                    if (active == 0) list.scrollUp();
                    if (active == 2) tree.scrollUp();
                    if (active == 3) table.scrollUp();
                }
                if (id == "<Down>") {
                    if (active == 0) list.scrollDown();
                    if (active == 2) tree.scrollDown();
                    if (active == 3) table.scrollDown();
                }
                if (id == "<Enter>" && mode == Mode::Expanded && active == 2)
                    tree.toggleExpand();
            }

            // ── Mouse ─────────────────────────────────────────────────────────
            if (e.type == tui::EventType::Mouse) {
                int mx = e.mouse.x, my = e.mouse.y;

                if (e.id == "<MouseLeft>") {
                    if (mode == Mode::Tiled) {
                        // Check expand buttons first.
                        bool handled = false;
                        for (int i = 0; i < 4 && !handled; ++i) {
                            if (hitBtn(mx, my, expandPos(panels[i]))) {
                                expand(i);
                                handled = true;
                            }
                        }
                        // Otherwise focus + row-select.
                        if (!handled) {
                            for (int i = 0; i < 4; ++i) {
                                if (panels[i].widget->rect.contains({mx, my})) {
                                    switchTo(i);
                                    int off = my - panels[i].widget->inner.min.y;
                                    if (i == 0 && off >= 0) list.selectVisibleRow(off);
                                    if (i == 2 && off >= 0) tree.selectVisibleRow(off);
                                    if (i == 3 && off >= 0) table.selectVisibleRow(off);
                                    break;
                                }
                            }
                        }
                    } else {
                        // Collapse button.
                        if (hitBtn(mx, my, collapsePos(cols))) {
                            collapse();
                        }
                        // Tab bar — detect which tab was clicked.
                        else if (my < TAB_H) {
                            int x = tabs.inner.min.x;
                            for (int i = 0; i < 4; ++i) {
                                int w = static_cast<int>(panels[i].name.size());
                                if (mx >= x && mx < x + w) { switchTo(i); break; }
                                x += w + 2;  // name + "│ " gap
                            }
                        }
                        // Content area click-to-select.
                        else {
                            int off = my - panels[active].widget->inner.min.y;
                            if (active == 0 && off >= 0) list.selectVisibleRow(off);
                            if (active == 2 && off >= 0) tree.selectVisibleRow(off);
                            if (active == 3 && off >= 0) table.selectVisibleRow(off);
                        }
                    }
                }

                // Scroll wheel — use the widget under the cursor.
                auto target = [&]() -> int {
                    if (mode == Mode::Expanded) return active;
                    for (int i = 0; i < 4; ++i)
                        if (panels[i].widget->rect.contains({mx, my})) return i;
                    return active;
                };
                if (e.id == "<MouseWheelUp>") {
                    switch (target()) {
                        case 0: list.scrollUp();   break;
                        case 2: tree.scrollUp();   break;
                        case 3: table.scrollUp();  break;
                    }
                }
                if (e.id == "<MouseWheelDown>") {
                    switch (target()) {
                        case 0: list.scrollDown();  break;
                        case 2: tree.scrollDown();  break;
                        case 3: table.scrollDown(); break;
                    }
                }
            }

            // ── Resize ────────────────────────────────────────────────────────
            if (e.type == tui::EventType::Resize) {
                cols = e.resize.width;
                rows = e.resize.height;
                tabs.setRect(0, 0, cols, TAB_H);
                if (mode == Mode::Tiled) doTiledLayout(panels, cols, rows);
                else                     doExpandedLayout(panels[active], cols, rows);
            }
        }

        tui::Buffer frame(tui::makeRect(0, 0, cols, rows));
        drawFrame(frame, panels, tabs, mode, active, cols);
    }
}
