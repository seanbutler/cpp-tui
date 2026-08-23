#include <tui/tui.hpp>
#include <cmath>
#include <chrono>
#include <thread>
#include <string>
#include <vector>

// ── Focus styling ─────────────────────────────────────────────────────────────
// Focused pane gets a cyan border; all others stay white.

static void setFocus(std::vector<tui::Block*>& panes, int focused) {
    for (int i = 0; i < static_cast<int>(panes.size()); ++i) {
        panes[i]->borderStyle = (i == focused)
            ? tui::newStyle(tui::ColorCyan)
            : tui::newStyle(tui::ColorWhite);
        panes[i]->titleStyle = (i == focused)
            ? tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold)
            : tui::newStyle(tui::ColorWhite);
    }
}

// ── Hit test — which pane contains the mouse point? ──────────────────────────

static int hitTest(const std::vector<tui::Block*>& panes, int mx, int my) {
    for (int i = 0; i < static_cast<int>(panes.size()); ++i)
        if (panes[i]->rect.contains({mx, my}))
            return i;
    return -1;
}

// ── Sine wave helper ──────────────────────────────────────────────────────────

static std::vector<double> sine(int n, double phase) {
    std::vector<double> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = std::sin(phase + i * 0.15) * 0.5 + 0.5;
    return v;
}

// ── Layout helper — divide terminal into a 2×2 grid ──────────────────────────

static void layout(std::vector<tui::Block*>& panes, int cols, int rows) {
    int hw = cols / 2, hh = rows / 2;
    panes[0]->setRect(0,  0,  hw,   hh);    // top-left
    panes[1]->setRect(hw, 0,  cols, hh);    // top-right
    panes[2]->setRect(0,  hh, hw,   rows);  // bottom-left
    panes[3]->setRect(hw, hh, cols, rows);  // bottom-right
}

// ─────────────────────────────────────────────────────────────────────────────

int main() {
    tui::init();
    auto [cols, rows] = tui::terminalSize();

    // ── Pane 0 — List (top-left) ──────────────────────────────────────────────
    tui::widgets::List list;
    list.title = "List  [0]";
    list.rows  = {
        "Aardvark",   "Baboon",     "Capybara",  "Dingo",
        "Elephant",   "Flamingo",   "Gecko",     "Hamster",
        "Iguana",     "Jellyfish",  "Kangaroo",  "Lemur",
        "Manatee",    "Narwhal",    "Octopus",   "Pangolin",
        "Quokka",     "Raccoon",    "Salamander","Tapir",
    };
    list.selectedRowStyle = tui::newStyle(tui::ColorBlack, tui::ColorCyan);

    // ── Pane 1 — Plot (top-right) ─────────────────────────────────────────────
    tui::widgets::Plot plot;
    plot.title      = "Plot  [1]";
    plot.lineColors = {tui::ColorGreen, tui::ColorYellow, tui::ColorRed};

    // ── Pane 2 — Tree (bottom-left) ───────────────────────────────────────────
    std::vector<tui::widgets::TreeNode> tn(20);
    tn[0].label = "Documents";
    tn[1].label = "report.pdf";
    tn[2].label = "notes.txt";
    tn[3].label = "Projects";
    tn[4].label = "tui-cpp";
    tn[5].label = "src";
    tn[6].label = "buffer.cpp";
    tn[7].label = "terminal.cpp";
    tn[8].label = "include";
    tn[9].label = "tui.hpp";
    tn[10].label = "other-project";
    tn[11].label = "main.cpp";
    tn[12].label = "Downloads";
    tn[13].label = "archive.zip";
    tn[14].label = "photo.jpg";

    tn[5].children  = {&tn[6], &tn[7]};
    tn[8].children  = {&tn[9]};
    tn[4].children  = {&tn[5], &tn[8]};
    tn[10].children = {&tn[11]};
    tn[3].children  = {&tn[4], &tn[10]};
    tn[0].children  = {&tn[1], &tn[2]};
    tn[12].children = {&tn[13], &tn[14]};

    tn[0].expanded  = true;
    tn[3].expanded  = true;
    tn[4].expanded  = true;
    tn[5].expanded  = true;
    tn[12].expanded = true;

    tui::widgets::Tree tree;
    tree.title            = "Tree  [2]";
    tree.selectedRowStyle = tui::newStyle(tui::ColorBlack, tui::ColorCyan);
    tree.setNodes({&tn[0], &tn[3], &tn[12]});

    // ── Pane 3 — Table (bottom-right) ────────────────────────────────────────
    tui::widgets::Table table;
    table.title = "Table  [3]";
    table.rows  = {
        {"Language", "Paradigm",    "Year"},
        {"C++",      "Multi",       "1983"},
        {"Rust",     "Systems",     "2010"},
        {"Go",       "Concurrent",  "2009"},
        {"Python",   "Dynamic",     "1991"},
        {"Haskell",  "Functional",  "1990"},
        {"Zig",      "Systems",     "2016"},
        {"Odin",     "Imperative",  "2016"},
        {"Nim",      "Multi",       "2008"},
        {"Julia",    "Scientific",  "2012"},
        {"Swift",    "Multi",       "2014"},
        {"Kotlin",   "Multi",       "2011"},
    };
    table.rowStyles[0] = tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold);

    // ── Collect pane pointers for focus/layout management ────────────────────
    std::vector<tui::Block*> panes = {&list, &plot, &tree, &table};
    int focused = 0;
    setFocus(panes, focused);
    layout(panes, cols, rows);

    auto renderAll = [&]() {
        tui::render({&list, &plot, &tree, &table});
    };

    // ── Tip paragraph rendered once at the very top (1 row, no border) ───────
    // We just use render directly; the tip scrolls under pane 0 border so we
    // simply put a hint in the titles instead.

    double phase = 0.0;
    plot.data = { sine(80, phase), sine(80, phase + 1.0), sine(80, phase + 2.0) };
    renderAll();

    // ── Event loop ────────────────────────────────────────────────────────────
    auto events = tui::pollEvents();

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        phase += 0.12;
        plot.data = { sine(80, phase), sine(80, phase + 1.0), sine(80, phase + 2.0) };

        tui::Event e;
        while (events->tryPop(e)) {

            // ── Mouse ─────────────────────────────────────────────────────────
            if (e.type == tui::EventType::Mouse) {
                int hit = hitTest(panes, e.mouse.x, e.mouse.y);

                if (e.id == "<MouseLeft>" && hit >= 0) {
                    // Click → focus the pane that was clicked.
                    focused = hit;
                    setFocus(panes, focused);

                    // On List: clicking a visible row selects it.
                    if (focused == 0) {
                        int visibleOffset = e.mouse.y - list.inner.min.y;
                        if (visibleOffset >= 0)
                            list.selectVisibleRow(visibleOffset);
                    }
                    // On Tree: clicking a visible row selects it.
                    if (focused == 2) {
                        int visibleOffset = e.mouse.y - tree.inner.min.y;
                        if (visibleOffset >= 0)
                            tree.selectVisibleRow(visibleOffset);
                    }
                }

                // Scroll wheel — applies to whichever pane the cursor is over.
                if (e.id == "<MouseWheelUp>") {
                    int target = (hit >= 0) ? hit : focused;
                    if (target == 0) list.scrollUp();
                    if (target == 2) tree.scrollUp();
                }
                if (e.id == "<MouseWheelDown>") {
                    int target = (hit >= 0) ? hit : focused;
                    if (target == 0) list.scrollDown();
                    if (target == 2) tree.scrollDown();
                }
            }

            // ── Keyboard (acts on focused pane) ───────────────────────────────
            if (e.type == tui::EventType::Keyboard) {
                const auto& id = e.id;

                if (id == "q" || id == "<C-c>") {
                    events->stop();
                    tui::close();
                    return 0;
                }

                // Tab cycles focus: 0 → 1 → 2 → 3 → 0
                if (id == "<Tab>") {
                    focused = (focused + 1) % 4;
                    setFocus(panes, focused);
                }

                // Arrow keys — navigate within the focused pane.
                if (id == "<Up>") {
                    if (focused == 0) list.scrollUp();
                    if (focused == 2) tree.scrollUp();
                }
                if (id == "<Down>") {
                    if (focused == 0) list.scrollDown();
                    if (focused == 2) tree.scrollDown();
                }
                if (id == "<Enter>" && focused == 2) {
                    tree.toggleExpand();
                }
            }

            // ── Resize ────────────────────────────────────────────────────────
            if (e.type == tui::EventType::Resize) {
                cols = e.resize.width;
                rows = e.resize.height;
                layout(panes, cols, rows);
            }
        }

        renderAll();
    }
}
