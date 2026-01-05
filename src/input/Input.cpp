#include "input/Input.h"

#include "core/Utils.h"

InputState PollInput(bool& running, bool text_input_active) {
    static bool last_text_input_active = false;
    
    if (text_input_active != last_text_input_active) {
        if (text_input_active) {
            SDL_StartTextInput();
        } else {
            SDL_StopTextInput();
        }
        last_text_input_active = text_input_active;
    }

    InputState input{};
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
        if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            running = false;
        }
        if (e.type == SDL_MOUSEWHEEL) {
            input.mouse_wheel += e.wheel.y;
        }
        if (e.type == SDL_TEXTINPUT) {
            input.text_input += e.text.text;
        }
        if (e.type == SDL_KEYDOWN || e.type == SDL_KEYUP) {
            bool down = e.key.state == SDL_PRESSED;
            if (down && !e.key.repeat) {
                if (e.key.keysym.sym == SDLK_ESCAPE) {
                    input.text_escape = true;
                }
                if (e.key.keysym.sym == SDLK_F11) {
                    input.toggle_fullscreen = true;
                }
                if (e.key.keysym.sym == SDLK_F9) {
                    input.toggle_colliders = true;
                }
                if (e.key.keysym.sym == SDLK_BACKQUOTE) {
                    input.toggle_console = true;
                }
                if (e.key.keysym.sym == SDLK_t) {
                    input.toggle_chat = true;
                }
                if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    input.text_backspace = true;
                }
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER) {
                    input.text_enter = true;
                }
            }
        }
    }

    const Uint8* state = SDL_GetKeyboardState(nullptr);
    input.move_forward = (state[SDL_SCANCODE_W] ? 1.0f : 0.0f) - (state[SDL_SCANCODE_S] ? 1.0f : 0.0f);
    input.move_right = (state[SDL_SCANCODE_D] ? 1.0f : 0.0f) - (state[SDL_SCANCODE_A] ? 1.0f : 0.0f);
    input.jump = state[SDL_SCANCODE_SPACE];
    input.sprint = state[SDL_SCANCODE_LSHIFT] || state[SDL_SCANCODE_RSHIFT];
    input.crouch = state[SDL_SCANCODE_LCTRL] || state[SDL_SCANCODE_RCTRL];
    return input;
}
