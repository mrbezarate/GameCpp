#pragma once

#include <SDL.h>
#include <string>

struct InputState {
    float move_forward = 0.0f;
    float move_right = 0.0f;
    bool jump = false;
    bool sprint = false;
    bool crouch = false;
    bool toggle_fullscreen = false;
    bool toggle_console = false;
    bool toggle_chat = false;
    bool toggle_colliders = false;
    bool text_backspace = false;
    bool text_enter = false;
    bool text_escape = false;
    std::string text_input;
    int mouse_wheel = 0;
};

InputState PollInput(bool& running, bool text_input_active);
