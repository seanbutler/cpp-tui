#pragma once
#include <string>

namespace tui {

// Decode a UTF-8 string into a UTF-32 string.
// Invalid sequences are replaced with U'?'.
inline std::u32string utf8ToU32(const std::string& s) {
    std::u32string out;
    out.reserve(s.size());

    const unsigned char* p   = reinterpret_cast<const unsigned char*>(s.data());
    const unsigned char* end = p + s.size();

    while (p < end) {
        char32_t cp = 0;
        unsigned char b = *p++;

        if (b < 0x80) {
            // 1-byte sequence (ASCII)
            cp = b;
        } else if ((b & 0xE0) == 0xC0) {
            // 2-byte sequence
            cp = b & 0x1F;
            if (p < end) cp = (cp << 6) | (*p++ & 0x3F);
        } else if ((b & 0xF0) == 0xE0) {
            // 3-byte sequence
            cp = b & 0x0F;
            if (p < end) cp = (cp << 6) | (*p++ & 0x3F);
            if (p < end) cp = (cp << 6) | (*p++ & 0x3F);
        } else if ((b & 0xF8) == 0xF0) {
            // 4-byte sequence
            cp = b & 0x07;
            if (p < end) cp = (cp << 6) | (*p++ & 0x3F);
            if (p < end) cp = (cp << 6) | (*p++ & 0x3F);
            if (p < end) cp = (cp << 6) | (*p++ & 0x3F);
        } else {
            cp = U'?';  // invalid lead byte
        }

        out += cp;
    }
    return out;
}

} // namespace tui
