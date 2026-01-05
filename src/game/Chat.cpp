#include "game/Chat.h"

#include <algorithm>

void AddChatMessage(Chat& chat, const std::string& text, double time_sec) {
    if (text.empty()) {
        return;
    }
    ChatMessage msg{};
    msg.text = text;
    msg.time = time_sec;
    chat.messages.push_back(std::move(msg));
    if (chat.messages.size() > chat.max_lines) {
        size_t remove = chat.messages.size() - chat.max_lines;
        chat.messages.erase(chat.messages.begin(), chat.messages.begin() + static_cast<long long>(remove));
    }
}

float ChatAlpha(double age, double fade_start, double fade_end) {
    if (age <= fade_start) {
        return 1.0f;
    }
    if (age >= fade_end) {
        return 0.0f;
    }
    double t = (age - fade_start) / (fade_end - fade_start);
    return static_cast<float>(1.0 - t);
}


