#pragma once
#include "drawable.hpp"
#include <utility>
#include <initializer_list>
#include <vector>

namespace tui {

// Initialise ncurses.  Call once at program start.
void init();

// Tear down ncurses.  Call before the program exits.
void close();

// Returns (width, height) of the current terminal window.
std::pair<int, int> terminalSize();

// Render one or more widgets to the screen.
// Each widget's draw() is called to fill a Buffer, then the buffer is
// flushed to ncurses and the screen is refreshed once at the end.
void render(std::initializer_list<Drawable*> items);
void render(const std::vector<Drawable*>& items);

// Flush a pre-composed Buffer directly to the screen.
// Use this when you need to draw custom decorations on top of widgets
// (e.g. overlay buttons) before a single refresh().
void flushBuffer(const Buffer& buf);
void screenRefresh();  // call refresh() after one or more flushBuffer() calls

} // namespace tui
