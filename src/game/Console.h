#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

struct Game;

struct Console {
    std::vector<std::string> lines;
    std::string input;
    bool open = false;
    size_t max_lines = 64;
    
    std::unordered_map<std::string, std::function<void(Game&, const std::vector<std::string>&)>> commands;
};

void InitConsole(Console& console);
void ExecuteCommand(Console& console, Game& game, const std::string& command_line);
void AddLine(Console& console, const std::string& text);

