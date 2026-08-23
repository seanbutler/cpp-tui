#include "tui/events.hpp"

// ncurses headers — must come before any macro conflicts.
#define _XOPEN_SOURCE_EXTENDED 1
#include <ncurses.h>

#include <string>
#include <unordered_map>

namespace tui {

// ── EventChannel ────────────────────────────────────────────────────────────

Event EventChannel::pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return !queue_.empty() || stopped_; });
    if (stopped_ && queue_.empty()) return Event{EventType::Unknown, "<stopped>"};
    Event e = queue_.front();
    queue_.pop();
    return e;
}

bool EventChannel::tryPop(Event& out) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) return false;
    out = queue_.front();
    queue_.pop();
    return true;
}

void EventChannel::push(Event e) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(std::move(e));
    }
    cv_.notify_one();
}

void EventChannel::stop() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopped_ = true;
    }
    cv_.notify_all();
}

// ── Key mapping ─────────────────────────────────────────────────────────────

// Maps ncurses KEY_* codes to termui-style id strings.
static const std::unordered_map<int, std::string> KEY_MAP = {
    {KEY_F(1),        "<F1>"},
    {KEY_F(2),        "<F2>"},
    {KEY_F(3),        "<F3>"},
    {KEY_F(4),        "<F4>"},
    {KEY_F(5),        "<F5>"},
    {KEY_F(6),        "<F6>"},
    {KEY_F(7),        "<F7>"},
    {KEY_F(8),        "<F8>"},
    {KEY_F(9),        "<F9>"},
    {KEY_F(10),       "<F10>"},
    {KEY_F(11),       "<F11>"},
    {KEY_F(12),       "<F12>"},
    {KEY_IC,          "<Insert>"},
    {KEY_DC,          "<Delete>"},
    {KEY_HOME,        "<Home>"},
    {KEY_END,         "<End>"},
    {KEY_PPAGE,       "<PageUp>"},
    {KEY_NPAGE,       "<PageDown>"},
    {KEY_UP,          "<Up>"},
    {KEY_DOWN,        "<Down>"},
    {KEY_LEFT,        "<Left>"},
    {KEY_RIGHT,       "<Right>"},
    {KEY_BACKSPACE,   "<Backspace>"},
    {'\t',            "<Tab>"},
    {'\n',            "<Enter>"},
    {'\r',            "<Enter>"},
    {27,              "<Escape>"},   // ESC key
    {' ',             "<Space>"},
    // Control keys: ncurses gives these as small integers 1–26.
    {1,  "<C-a>"}, {2,  "<C-b>"}, {3,  "<C-c>"}, {4,  "<C-d>"},
    {5,  "<C-e>"}, {6,  "<C-f>"}, {7,  "<C-g>"}, {11, "<C-k>"},
    {12, "<C-l>"}, {14, "<C-n>"}, {15, "<C-o>"}, {16, "<C-p>"},
    {17, "<C-q>"}, {18, "<C-r>"}, {19, "<C-s>"}, {20, "<C-t>"},
    {21, "<C-u>"}, {22, "<C-v>"}, {23, "<C-w>"}, {24, "<C-x>"},
    {25, "<C-y>"}, {26, "<C-z>"},
};

static Event makeKeyEvent(int key) {
    Event e;
    e.type = EventType::Keyboard;

    auto it = KEY_MAP.find(key);
    if (it != KEY_MAP.end()) {
        e.id = it->second;
    } else if (key >= 32 && key < 127) {
        // Printable ASCII
        e.id = std::string(1, static_cast<char>(key));
    } else {
        e.id = "<unknown:" + std::to_string(key) + ">";
    }

    return e;
}

// ── pollEvents ──────────────────────────────────────────────────────────────

std::shared_ptr<EventChannel> pollEvents() {
    auto ch = std::make_shared<EventChannel>();

    std::thread([ch]() {
        // Block for up to 100 ms at a time so we can honour stop().
        wtimeout(stdscr, 100);

        while (!ch->stopped()) {
            int key = wgetch(stdscr);

            if (key == ERR) {
                // Timeout — loop back and check stopped flag.
                continue;
            }

            if (key == KEY_RESIZE) {
                // Terminal was resized.
                int rows, cols;
                getmaxyx(stdscr, rows, cols);
                Event e;
                e.type          = EventType::Resize;
                e.id            = "<Resize>";
                e.resize.width  = cols;
                e.resize.height = rows;
                ch->push(e);
                continue;
            }

            if (key == KEY_MOUSE) {
                MEVENT me;
                if (getmouse(&me) == OK) {
                    Event e;
                    e.type    = EventType::Mouse;
                    e.mouse.x = me.x;
                    e.mouse.y = me.y;
                    e.mouse.drag = (me.bstate & REPORT_MOUSE_POSITION) != 0;

                    if      (me.bstate & BUTTON1_CLICKED)  e.id = "<MouseLeft>";
                    else if (me.bstate & BUTTON3_CLICKED)  e.id = "<MouseRight>";
                    else if (me.bstate & BUTTON2_CLICKED)  e.id = "<MouseMiddle>";
                    else if (me.bstate & BUTTON4_PRESSED)  e.id = "<MouseWheelUp>";
                    else if (me.bstate & BUTTON5_PRESSED)  e.id = "<MouseWheelDown>";
                    else e.id = "<Mouse>";

                    ch->push(e);
                }
                continue;
            }

            ch->push(makeKeyEvent(key));
        }
    }).detach();

    return ch;
}

} // namespace tui
