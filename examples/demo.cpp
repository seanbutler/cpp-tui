#include <tui/tui.hpp>
#include <chrono>
#include <thread>

int main() {
    tui::init();

    auto [cols, rows] = tui::terminalSize();
    int hw = cols / 2;
    int hh = rows / 2;

    // ── Paragraph (top-left) ───────────────────────────────────────────────
    tui::widgets::Paragraph para;
    para.title = "Paragraph";
    para.text  = "tui-cpp demo\n\nA C++ port of termui.\n\nUse <q> to quit.";
    para.setRect(0, 0, hw, hh);

    // ── List (top-right) ───────────────────────────────────────────────────
    tui::widgets::List list;
    list.title = "List";
    list.rows  = {"Item 1", "Item 2", "Item 3", "Item 4",
                  "Item 5", "Item 6", "Item 7", "Item 8"};
    list.selectedRowStyle = tui::newStyle(tui::ColorYellow);
    list.setRect(hw, 0, cols, hh);

    // ── BarChart (bottom-left) ─────────────────────────────────────────────
    tui::widgets::BarChart bar;
    bar.title  = "BarChart";
    bar.data   = {3, 6, 4, 9, 2, 7, 5};
    bar.labels = {"A", "B", "C", "D", "E", "F", "G"};
    bar.setRect(0, hh, hw, rows);

    // ── Gauge (bottom-right) ───────────────────────────────────────────────
    tui::widgets::Gauge gauge;
    gauge.title    = "Gauge";
    gauge.percent  = 42;
    gauge.barColor = tui::ColorGreen;
    gauge.setRect(hw, hh, cols, rows);

    // Initial render.
    tui::render({&para, &list, &bar, &gauge});

    // ── Event loop ─────────────────────────────────────────────────────────
    auto events = tui::pollEvents();
    int  tick   = 0;

    while (true) {
        tui::Event e;
        // Poll for an event with a short timeout so we can animate.
        if (events->tryPop(e)) {
            if (e.type == tui::EventType::Keyboard) {
                if (e.id == "q" || e.id == "<C-c>") break;
                if (e.id == "<Down>") list.scrollDown();
                if (e.id == "<Up>")   list.scrollUp();
            }
            if (e.type == tui::EventType::Resize) {
                // Refit widgets to the new terminal size.
                cols = e.resize.width;
                rows = e.resize.height;
                hw   = cols / 2;
                hh   = rows / 2;
                para.setRect(0,  0,  hw,   hh);
                list.setRect(hw, 0,  cols, hh);
                bar.setRect( 0,  hh, hw,   rows);
                gauge.setRect(hw, hh, cols, rows);
            }
        }

        // Animate the gauge bar every ~50 ms.
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        tick++;
        gauge.percent = (gauge.percent + 1) % 101;

        tui::render({&para, &list, &bar, &gauge});
    }

    events->stop();
    tui::close();
    return 0;
}
