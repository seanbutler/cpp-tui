#include <tui/tui.hpp>
#include <cmath>
#include <chrono>
#include <thread>
#include <string>

// Generate N samples of a sine wave with the given phase offset (range 0..1).
static std::vector<double> sineWave(int n, double phase, double amplitude = 1.0) {
    std::vector<double> v(n);
    for (int i = 0; i < n; ++i)
        v[i] = amplitude * (std::sin(phase + i * 0.2) * 0.5 + 0.5);
    return v;
}

int main() {
    tui::init();
    auto [cols, rows] = tui::terminalSize();

    // ── Plot: two animated sine waves ─────────────────────────────────────────
    tui::widgets::Plot plot;
    plot.title      = "Plot  (braille line chart — animated)";
    plot.lineColors = {tui::ColorGreen, tui::ColorYellow};

    // ── Table: implementation status ─────────────────────────────────────────
    tui::widgets::Table table;
    table.title = "Table  (press q to quit)";
    table.rows  = {
        // Row 0 = header (bold+cyan by default via rowStyles[0]).
        {"Widget",      "Lines", "Description"},
        {"Paragraph",   "~40",   "Text with optional word-wrap"},
        {"List",        "~50",   "Scrollable rows with selection"},
        {"BarChart",    "~50",   "Vertical bars with labels"},
        {"Gauge",       "~40",   "Horizontal progress bar"},
        {"Plot",        "~80",   "Braille line chart"},
        {"Table",       "~60",   "Rows + columns with separators"},
        {"Grid",        "~70",   "Ratio-based layout engine"},
    };
    table.rowStyles[0] = tui::newStyle(tui::ColorCyan, tui::ColorClear, tui::ModifierBold);

    // ── Grid: top 55% = plot, bottom 45% = table ──────────────────────────────
    // Grid calls setRect + draw on each child into one shared buffer,
    // so we only need to render the grid itself.
    tui::Grid grid;
    grid.set({
        tui::gridRow(0.55, { tui::gridCol(1.0, &plot)  }),
        tui::gridRow(0.45, { tui::gridCol(1.0, &table) }),
    });
    grid.setRect(0, 0, cols, rows);

    tui::render({&grid});

    // ── Event loop ────────────────────────────────────────────────────────────
    auto events = tui::pollEvents();
    double phase = 0.0;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        phase += 0.15;

        plot.data = { sineWave(60, phase, 1.0), sineWave(60, phase + 1.5, 0.7) };

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
                grid.setRect(0, 0, cols, rows);
            }
        }

        tui::render({&grid});
    }
}
