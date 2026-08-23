#include <tui/tui.hpp>
#include <cmath>
#include <chrono>
#include <thread>
#include <string>
#include <numeric>

// ── helpers ───────────────────────────────────────────────────────────────────

static std::vector<double> sineData(int n, double phase, double lo, double hi) {
    std::vector<double> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = lo + (hi - lo) * (std::sin(phase + i * 0.25) * 0.5 + 0.5);
    return v;
}

// ── build the static tree once ───────────────────────────────────────────────

static std::vector<tui::widgets::TreeNode> makeTreeStorage() {
    // We store nodes in a vector so pointers stay stable.
    return std::vector<tui::widgets::TreeNode>(20);
}

// ── main ─────────────────────────────────────────────────────────────────────

int main() {
    tui::init();
    auto [cols, rows] = tui::terminalSize();

    // ── Tabs ─────────────────────────────────────────────────────────────────
    tui::widgets::TabPane tabs({"Sparkline", "StackedBar", "PieChart", "Tree", "StyleParser"});
    tabs.title = "";
    tabs.border = false;

    // ── Sparkline group ───────────────────────────────────────────────────────
    tui::widgets::Sparkline slCPU, slMem;
    slCPU.title     = "CPU";
    slCPU.lineColor = tui::ColorGreen;
    slMem.title     = "Memory";
    slMem.lineColor = tui::ColorYellow;

    tui::widgets::SparklineGroup sparkGroup({&slCPU, &slMem});
    sparkGroup.title = "Sparkline";

    // ── Stacked bar chart ─────────────────────────────────────────────────────
    tui::widgets::StackedBarChart stacked;
    stacked.title  = "StackedBarChart  (user / sys / io)";
    stacked.labels = {"A","B","C","D","E","F","G","H"};
    stacked.barWidth = 5;
    stacked.barGap   = 1;

    // ── Pie chart ─────────────────────────────────────────────────────────────
    tui::widgets::PieChart pie;
    pie.title = "PieChart";
    pie.data  = {30, 20, 15, 25, 10};
    pie.labelFormatter = [](int i, double v) {
        char buf[8];
        std::snprintf(buf, sizeof(buf), "%.0f%%", v);
        return std::string(buf);
    };

    // ── Tree ──────────────────────────────────────────────────────────────────
    // Allocate nodes in flat arrays so pointers are stable.
    std::vector<tui::widgets::TreeNode> nodes(18);
    //  src/
    nodes[0].label    = "src";
    nodes[1].label    = "widgets";
    nodes[2].label    = "paragraph.cpp";
    nodes[3].label    = "list.cpp";
    nodes[4].label    = "barchart.cpp";
    nodes[5].label    = "buffer.cpp";
    nodes[6].label    = "terminal.cpp";
    //  include/
    nodes[7].label    = "include";
    nodes[8].label    = "tui";
    nodes[9].label    = "tui.hpp";
    nodes[10].label   = "block.hpp";
    nodes[11].label   = "buffer.hpp";
    //  examples/
    nodes[12].label   = "examples";
    nodes[13].label   = "hello_world.cpp";
    nodes[14].label   = "demo.cpp";
    nodes[15].label   = "demo2.cpp";
    nodes[16].label   = "demo3.cpp";
    //  CMakeLists.txt
    nodes[17].label   = "CMakeLists.txt";

    // Wire up children.
    nodes[1].children = {&nodes[2], &nodes[3], &nodes[4]};
    nodes[0].children = {&nodes[1], &nodes[5], &nodes[6]};
    nodes[8].children = {&nodes[9], &nodes[10], &nodes[11]};
    nodes[7].children = {&nodes[8]};
    nodes[12].children = {&nodes[13], &nodes[14], &nodes[15], &nodes[16]};

    // Start all expanded.
    nodes[0].expanded = true;
    nodes[1].expanded = true;
    nodes[7].expanded = true;
    nodes[8].expanded = true;
    nodes[12].expanded = true;

    tui::widgets::Tree tree;
    tree.title            = "Tree  (↑↓ navigate, Enter toggle)";
    tree.selectedRowStyle = tui::newStyle(tui::ColorYellow);
    tree.setNodes({&nodes[0], &nodes[7], &nodes[12], &nodes[17]});

    // ── StyleParser demo (Paragraph with inline markup) ───────────────────────
    tui::widgets::Paragraph stylePara;
    stylePara.title    = "StyleParser";
    stylePara.wrapText = false;
    stylePara.text =
        "[tui-cpp](fg:cyan,mod:bold) inline style markup:\n"
        "\n"
        "[red text](fg:red)   "
        "[green](fg:green)   "
        "[yellow](fg:yellow)\n"
        "\n"
        "[bold](mod:bold)   "
        "[underline](mod:underline)   "
        "[reverse](mod:reverse)\n"
        "\n"
        "[blue on white](fg:blue,bg:white)   "
        "[magenta bold](fg:magenta,mod:bold)\n"
        "\n"
        "Syntax: [text](fg:color,bg:color,mod:modifier)";

    // ── Grid: tab bar (3 rows) on top, content area below ────────────────────
    // We render the tab bar and content separately since Grid doesn't support
    // conditional rendering — the active tab widget changes each frame.
    tabs.setRect(0, 0, cols, 3);

    auto contentRect = [&](int c, int r) {
        return tui::makeRect(0, 3, c, r);
    };

    auto applyRect = [&](tui::Drawable& w) {
        auto r = contentRect(cols, rows);
        w.setRect(r.min.x, r.min.y, r.max.x, r.max.y);
    };

    applyRect(sparkGroup);
    applyRect(stacked);
    applyRect(pie);
    applyRect(tree);
    applyRect(stylePara);

    auto renderActive = [&]() {
        tui::render({&tabs});
        switch (tabs.activeTabIndex) {
            case 0: tui::render({&sparkGroup}); break;
            case 1: tui::render({&stacked});    break;
            case 2: tui::render({&pie});         break;
            case 3: tui::render({&tree});        break;
            case 4: tui::render({&stylePara});   break;
        }
    };

    renderActive();

    // ── Event loop ────────────────────────────────────────────────────────────
    auto events = tui::pollEvents();
    double phase = 0.0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(80));
        phase += 0.2;

        // Animate sparklines.
        slCPU.data = sineData(cols, phase,      0,  100);
        slMem.data = sineData(cols, phase + 1.0, 20, 80);

        // Animate stacked bars.
        stacked.data.clear();
        for (int i = 0; i < 8; ++i) {
            double u = 20 + 30 * std::sin(phase + i * 0.4);
            double s = 10 + 15 * std::sin(phase + i * 0.7 + 1.0);
            double io = 5  + 10 * std::sin(phase + i * 0.3 + 2.0);
            stacked.data.push_back({std::abs(u), std::abs(s), std::abs(io)});
        }

        // Handle events.
        tui::Event e;
        while (events->tryPop(e)) {
            if (e.type == tui::EventType::Keyboard) {
                const auto& id = e.id;
                if (id == "q" || id == "<C-c>") {
                    events->stop();
                    tui::close();
                    return 0;
                }
                if (id == "<Left>")  tabs.focusLeft();
                if (id == "<Right>") tabs.focusRight();
                // Tree navigation (only active on tree tab).
                if (tabs.activeTabIndex == 3) {
                    if (id == "<Up>")    tree.scrollUp();
                    if (id == "<Down>")  tree.scrollDown();
                    if (id == "<Enter>") tree.toggleExpand();
                }
            }
            if (e.type == tui::EventType::Resize) {
                cols = e.resize.width;
                rows = e.resize.height;
                tabs.setRect(0, 0, cols, 3);
                applyRect(sparkGroup);
                applyRect(stacked);
                applyRect(pie);
                applyRect(tree);
                applyRect(stylePara);
            }
        }

        renderActive();
    }
}
