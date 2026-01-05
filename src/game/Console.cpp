#include "game/Console.h"

#include <sstream>
#include <algorithm>
#include <cmath>
#include <cctype>

#include "core/Log.h"
#include "game/Game.h"
#include "math/Vec3.h"

static void SplitCommand(const std::string& line, std::string& cmd, std::vector<std::string>& args) {
    std::istringstream iss(line);
    iss >> cmd;
    std::string arg;
    while (iss >> arg) {
        args.push_back(arg);
    }
}

void InitConsole(Console& console) {
    console.lines.clear();
    console.input.clear();
    console.open = false;
    
    // Built-in commands
    console.commands["clear"] = [&console](Game&, const std::vector<std::string>&) {
        console.lines.clear();
        AddLine(console, "Console cleared.");
    };
    
    console.commands["help"] = [&console](Game&, const std::vector<std::string>&) {
        AddLine(console, "Available commands:");
        AddLine(console, "  clear - Clear console");
        AddLine(console, "  help - Show this help");
        AddLine(console, "  fps - Toggle FPS display");
        AddLine(console, "  noclip - Toggle noclip mode");
        AddLine(console, "  vh - Toggle spectator mode (same as noclip)");
        AddLine(console, "  teleport <x> <y> <z> - Teleport player");
        AddLine(console, "  quit - Exit game");
    };
    
    console.commands["fps"] = [&console](Game& game, const std::vector<std::string>&) {
        game.show_fps = !game.show_fps;
        AddLine(console, game.show_fps ? "FPS display enabled." : "FPS display disabled.");
    };
    
    console.commands["noclip"] = [&console](Game& game, const std::vector<std::string>&) {
        game.noclip = !game.noclip;
        AddLine(console, game.noclip ? "Noclip enabled." : "Noclip disabled.");
    };
    
    console.commands["vh"] = [&console](Game& game, const std::vector<std::string>&) {
        game.noclip = !game.noclip;
        AddLine(console, game.noclip ? "Spectator mode enabled." : "Spectator mode disabled.");
    };
    
    console.commands["teleport"] = [&console](Game& game, const std::vector<std::string>& args) {
        if (args.size() != 3) {
            AddLine(console, "Usage: teleport <x> <y> <z>");
            return;
        }
        try {
            float x = std::stof(args[0]);
            float y = std::stof(args[1]);
            float z = std::stof(args[2]);
            game.player.pos = Vec3{x, y, z};
            AddLine(console, "Teleported to (" + args[0] + ", " + args[1] + ", " + args[2] + ")");
        } catch (...) {
            AddLine(console, "Invalid coordinates.");
        }
    };
    
    console.commands["quit"] = [](Game&, const std::vector<std::string>&) {
        // Will be handled by main loop
    };
}

void ExecuteCommand(Console& console, Game& game, const std::string& command_line) {
    if (command_line.empty()) {
        return;
    }
    
    std::string cmd;
    std::vector<std::string> args;
    SplitCommand(command_line, cmd, args);
    
    // Convert to lowercase
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    
    auto it = console.commands.find(cmd);
    if (it != console.commands.end()) {
        it->second(game, args);
    } else {
        AddLine(console, "Unknown command: " + cmd + ". Type 'help' for available commands.");
    }
}

void AddLine(Console& console, const std::string& text) {
    if (text.empty()) {
        return;
    }
    console.lines.push_back(text);
    if (console.lines.size() > console.max_lines) {
        size_t remove = console.lines.size() - console.max_lines;
        console.lines.erase(console.lines.begin(), console.lines.begin() + static_cast<long long>(remove));
    }
}

