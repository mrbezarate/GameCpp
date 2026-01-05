#pragma once

#include <cstddef>

struct Config {
    // Graphics
    int window_width = 1280;
    int window_height = 720;
    bool fullscreen = false;
    bool vsync = true;
    float fov = 60.0f;
    float mouse_sensitivity = 0.08f;
    
    // Player physics
    float walk_speed = 8.0f;
    float sprint_speed = 12.5f;
    float crouch_speed = 4.0f;
    float jump_velocity = 11.0f;
    float gravity = 28.0f;
    
    // Rendering
    bool post_process_enabled = true;
    float near_plane = 0.1f;
    float far_plane = 2000.0f;
    
    // Console
    size_t max_console_lines = 64;
    size_t max_chat_lines = 64;
    double chat_fade_start = 7.0;
    double chat_fade_end = 10.0;
};

Config LoadConfig();
void SaveConfig(const Config& config);


