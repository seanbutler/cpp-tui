#pragma once
#include "tui/cell.hpp"
#include <vector>
#include <string>

namespace tui {

// Parse a string that may contain inline style markup and return a flat
// list of styled cells ready to be written into a Buffer.
//
// Syntax:  [text to style](fg:color,bg:color,mod:modifier)
//
//   Colors:    red  green  yellow  blue  magenta  cyan  white  black  clear
//   Modifiers: bold  underline  reverse
//
// Anything outside the markup is rendered with defaultStyle.
// Unrecognised markup is emitted as literal characters.
//
// Examples:
//   "[hello](fg:red)"                   — red "hello"
//   "[bold text](mod:bold)"             — bold "bold text"
//   "[hi](fg:yellow,bg:blue,mod:bold)"  — yellow on blue, bold
//   "plain [styled](fg:green) plain"    — mixed
//
std::vector<Cell> parseStyles(const std::string& s, Style defaultStyle);

} // namespace tui
