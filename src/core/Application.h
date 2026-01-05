#pragma once

#include <SDL.h>

struct Game;
struct InputState;

struct Application {
    SDL_Window* window = nullptr;
    SDL_GLContext gl_context = nullptr;
    bool running = true;
    int window_width = 1280;
    int window_height = 720;
    bool vsync_enabled = false;
    float fps = 0.0f;
    double frame_time = 0.0;
};

bool InitApplication(Application& app);
void ShutdownApplication(Application& app);
void ProcessEvents(Application& app, Game& game, InputState& input);
void UpdateFPS(Application& app, double dt);


