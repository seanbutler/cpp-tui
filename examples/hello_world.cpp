#include <tui/tui.hpp>

int main() {
    tui::init();

    tui::widgets::Paragraph p;
    p.text = "Hello World!\n\nPress any key to exit.";
    p.title = "tui-cpp";
    p.setRect(0, 0, 30, 7);

    tui::render({&p});

    // Wait for any key press then quit.
    auto events = tui::pollEvents();
    events->pop();
    events->stop();

    tui::close();
    return 0;
}
