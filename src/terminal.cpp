#include "tui/terminal.hpp"
#include "tui/buffer.hpp"
#include "tui/style.hpp"

#define _XOPEN_SOURCE_EXTENDED 1
#include <ncurses.h>

#include <locale.h>
#include <unordered_map>
#include <cstdint>

namespace tui {

// ── Color pair management ────────────────────────────────────────────────────
// ncurses requires color pairs to be pre-initialised with init_pair().
// We lazily allocate a pair for each (fg, bg) combination we encounter.

static std::unordered_map<uint32_t, short> g_colorPairs;
static short g_nextPair = 1;  // pair 0 is the terminal default

// Convert a tui::Color to a ncurses short.
// ColorClear (-1) → -1 which ncurses treats as "default" (requires use_default_colors()).
static short toNcursesColor(Color c) {
    if (c == ColorClear) return -1;
    return static_cast<short>(c);
}

// Encode (fg, bg) into a single 32-bit key for the map.
static uint32_t pairKey(short fg, short bg) {
    return (static_cast<uint32_t>(static_cast<uint16_t>(fg)) << 16) |
           static_cast<uint16_t>(bg);
}

// Return (or create) the ncurses color pair for this fg/bg combination.
static short getColorPair(Color fg, Color bg) {
    short nfg = toNcursesColor(fg);
    short nbg = toNcursesColor(bg);
    uint32_t k = pairKey(nfg, nbg);

    auto it = g_colorPairs.find(k);
    if (it != g_colorPairs.end()) return it->second;

    if (g_nextPair >= COLOR_PAIRS) return 0;  // out of pairs, use default

    init_pair(g_nextPair, nfg, nbg);
    g_colorPairs[k] = g_nextPair;
    return g_nextPair++;
}

// Convert tui::Modifier flags to ncurses attribute bits.
static attr_t toNcursesAttr(Modifier mod) {
    attr_t a = A_NORMAL;
    if (mod & ModifierBold)      a |= A_BOLD;
    if (mod & ModifierUnderline) a |= A_UNDERLINE;
    if (mod & ModifierReverse)   a |= A_REVERSE;
    return a;
}

// ── Public API ───────────────────────────────────────────────────────────────

void init() {
    setlocale(LC_ALL, "");          // needed for Unicode output
    initscr();
    start_color();
    use_default_colors();           // allows color -1 = terminal default
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);                    // hide the cursor
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, nullptr);
}

void close() {
    endwin();
}

std::pair<int, int> terminalSize() {
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    return {cols, rows};
}

// Render a single Buffer to the ncurses virtual screen.
void flushBuffer(const Buffer& buf) {
    for (auto& [k, cell] : buf.cells()) {
        // Decode position from the key.
        int x = static_cast<int>(k >> 32);
        int y = static_cast<int>(static_cast<uint32_t>(k));

        if (!buf.rect.contains({x, y})) continue;

        short pair = getColorPair(cell.style.fg, cell.style.bg);
        attr_t attr = toNcursesAttr(cell.style.mod) | COLOR_PAIR(pair);

        // Build a ncurses complex character so we can handle Unicode.
        cchar_t cc{};
        wchar_t wch[CCHARW_MAX] = {};
        wch[0] = static_cast<wchar_t>(cell.rune);
        setcchar(&cc, wch, attr, pair, nullptr);

        mvadd_wch(y, x, &cc);
    }
}

void render(const std::vector<Drawable*>& items) {
    for (Drawable* item : items) {
        Buffer buf(item->getRect());
        item->lock();
        item->draw(buf);
        item->unlock();
        flushBuffer(buf);
    }
    refresh();
}

void render(std::initializer_list<Drawable*> items) {
    render(std::vector<Drawable*>(items));
}

void screenRefresh() {
    refresh();
}

} // namespace tui
