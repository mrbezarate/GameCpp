#include "core/Utils.h"

void PopUtf8(std::string& text) {
    if (text.empty()) {
        return;
    }
    size_t i = text.size() - 1;
    while (i > 0) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if ((c & 0xC0) != 0x80) {
            break;
        }
        --i;
    }
    text.erase(i);
}


