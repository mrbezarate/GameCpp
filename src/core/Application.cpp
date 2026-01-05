#include "core/Application.h"

#include <SDL.h>
#include <algorithm>

#include "core/Config.h"
#include "core/Log.h"
#include "core/Utils.h"
#include "game/Chat.h"
#include "game/Console.h"
#include "game/Game.h"
#include "input/Input.h"

bool InitApplication(Application& app) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS) != 0) {
        LogError("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    app.window = SDL_CreateWindow(
        "City.net offline slice",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        app.window_width, app.window_height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!app.window) {
        LogError("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    app.gl_context = SDL_GL_CreateContext(app.window);
    if (!app.gl_context) {
        LogError("SDL_GL_CreateContext failed: %s", SDL_GetError());
        SDL_DestroyWindow(app.window);
        SDL_Quit();
        return false;
    }

    app.vsync_enabled = SDL_GL_SetSwapInterval(1) == 0;
    if (!app.vsync_enabled) {
        SDL_GL_SetSwapInterval(0);
        LogInfo("VSync not available, using frame limiting");
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_GL_GetDrawableSize(app.window, &app.window_width, &app.window_height);
    
    LogInfo("Application initialized (%dx%d)", app.window_width, app.window_height);
    return true;
}

void ShutdownApplication(Application& app) {
    if (app.gl_context) {
        SDL_GL_DeleteContext(app.gl_context);
        app.gl_context = nullptr;
    }
    if (app.window) {
        SDL_DestroyWindow(app.window);
        app.window = nullptr;
    }
    SDL_Quit();
    LogInfo("Application shutdown");
}

void ProcessEvents(Application& app, Game& game, InputState& input) {
    bool text_active = game.console.open || game.chat.open;
    
    if (input.toggle_console) {
        game.console.open = !game.console.open;
        if (game.console.open) {
            game.chat.open = false;
        }
    }
    
    if (input.toggle_chat && !game.chat.open && !game.console.open) {
        game.chat.open = true;
        game.chat.scroll = 0;
    }
    
    if (input.text_escape) {
        if (game.console.open || game.chat.open) {
            game.console.open = false;
            game.chat.open = false;
            game.chat.scroll = 0;
        } else {
            app.running = false;
        }
    }
    
    if (input.text_enter && game.console.open) {
        if (!game.console.input.empty()) {
            std::string cmd = game.console.input;
            ExecuteCommand(game.console, game, cmd);
            // Check if quit command was executed
            std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
            if (cmd == "quit") {
                app.running = false;
            }
        }
        game.console.input.clear();
    }
    
    if (input.text_enter && game.chat.open) {
        if (!game.chat.input.empty()) {
            AddChatMessage(game.chat, game.chat.input, game.time_sec);
        }
        game.chat.input.clear();
        game.chat.open = false;
        game.chat.scroll = 0;
    }
    
    // Handle text input
    bool ui_text = game.console.open || game.chat.open;
    if (ui_text) {
        std::string& active_input = game.console.open ? game.console.input : game.chat.input;
        if (!input.text_input.empty()) {
            if (active_input.size() < 120) {
                active_input += input.text_input;
            }
        }
        if (input.text_backspace && !active_input.empty()) {
            PopUtf8(active_input);
        }
    }
    
    // Handle mouse wheel for chat scroll
    if (game.chat.open && input.mouse_wheel != 0) {
        int max_scroll = static_cast<int>(game.chat.messages.size()) - 8; // approximate
        if (max_scroll < 0) {
            max_scroll = 0;
        }
        game.chat.scroll += input.mouse_wheel;
        if (game.chat.scroll < 0) {
            game.chat.scroll = 0;
        } else if (game.chat.scroll > max_scroll) {
            game.chat.scroll = max_scroll;
        }
    }
    
    bool want_relative = !game.console.open && !game.chat.open;
    SDL_SetRelativeMouseMode(want_relative ? SDL_TRUE : SDL_FALSE);
    
    if (input.toggle_fullscreen) {
        Uint32 flags = SDL_GetWindowFlags(app.window);
        bool fullscreen = (flags & SDL_WINDOW_FULLSCREEN_DESKTOP) != 0;
        SDL_SetWindowFullscreen(app.window, fullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
        SDL_GL_GetDrawableSize(app.window, &app.window_width, &app.window_height);
    }

    if (input.toggle_colliders) {
        game.show_colliders = !game.show_colliders;
    }
}

void UpdateFPS(Application& app, double dt) {
    static double accumulator = 0.0;
    static int frame_count = 0;
    static const double update_interval = 0.5; // Update FPS every 0.5 seconds
    
    accumulator += dt;
    frame_count++;
    
    if (accumulator >= update_interval) {
        app.fps = static_cast<float>(frame_count / accumulator);
        app.frame_time = accumulator / frame_count * 1000.0; // ms
        accumulator = 0.0;
        frame_count = 0;
    }
}

