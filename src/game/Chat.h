#pragma once

#include <string>
#include <vector>

struct ChatMessage {
    std::string text;
    double time = 0.0;
};

struct Chat {
    std::vector<ChatMessage> messages;
    std::string input;
    bool open = false;
    int scroll = 0;
    size_t max_lines = 64;
    double fade_start = 7.0;
    double fade_end = 10.0;
};

void AddChatMessage(Chat& chat, const std::string& text, double time_sec);
float ChatAlpha(double age, double fade_start, double fade_end);


