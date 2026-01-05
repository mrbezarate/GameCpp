#include <SDL.h>
#include <SDL_opengl.h>

#include <cmath>

#include "core/Application.h"
#include "core/Log.h"
#include "core/Time.h"
#include "game/Game.h"
#include "input/Input.h"
#include "math/Vec3.h"
#include "render/PostProcess.h"
#include "render/UI.h"
#include "render/WorldRenderer.h"

int main(int argc, char** argv) {
    (void)argc; (void)argv;

    Application app{};
    if (!InitApplication(app)) {
        return 1;
    }

    Game game{};
    InitGame(game);

    WorldRenderer world_renderer{};
    if (!InitWorldRenderer(world_renderer)) {
        LogError("Failed to initialize world renderer");
        ShutdownApplication(app);
        return 1;
    }

    PostProcess post{};
    if (!InitPostProcess(post, app.window_width, app.window_height)) {
        LogWarning("Post-process disabled");
    }

    float cam_yaw = 0.0f;
    float cam_pitch = 0.0f;

    const double target_fps = 240.0;
    const double target_frame = 1.0 / target_fps;
    const double perf_freq = game.clock.freq;

    while (app.running) {
        Uint64 frame_start = SDL_GetPerformanceCounter();
        double frame_dt = TickClock(game.clock);
        UpdateFPS(app, frame_dt);

        bool text_active = game.console.open || game.chat.open;
        InputState input = PollInput(app.running, text_active);
        
        ProcessEvents(app, game, input);

        SDL_GL_GetDrawableSize(app.window, &app.window_width, &app.window_height);
        ResizePostProcess(post, app.window_width, app.window_height);

        // Update camera
        if (!game.console.open && !game.chat.open) {
            int mx = 0, my = 0;
            SDL_GetRelativeMouseState(&mx, &my);
            cam_yaw -= static_cast<float>(mx) * game.config.mouse_sensitivity;
            cam_pitch -= static_cast<float>(my) * game.config.mouse_sensitivity;
            cam_pitch = clampf(cam_pitch, -89.0f, 89.0f);
            if (cam_yaw > 360.0f || cam_yaw < -360.0f) {
                cam_yaw = std::fmod(cam_yaw, 360.0f);
            }
        }

        // Block movement when UI is open
        if (game.console.open || game.chat.open) {
            input.move_forward = 0.0f;
            input.move_right = 0.0f;
            input.jump = false;
            input.sprint = false;
            input.crouch = false;
        }

        // Update game
        UpdateGame(game, input, cam_yaw, cam_pitch, frame_dt);

        Vec3 cam_pos = GetCameraPos(game.player);

        // Render
        if (post.valid) {
            BeginPostProcess(post);
        }

        glClearColor(0.35f, 0.55f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        RenderWorld(world_renderer, game.world, app.window_width, app.window_height, cam_yaw, cam_pitch, cam_pos, game.show_colliders);

        if (post.valid) {
            EndPostProcess(post, static_cast<float>(game.time_sec), game.config.near_plane, game.config.far_plane);
        }

        RenderUI(app, game);

        SDL_GL_SwapWindow(app.window);

        // Frame limiting
        if (!app.vsync_enabled) {
            double elapsed = (SDL_GetPerformanceCounter() - frame_start) / perf_freq;
            if (elapsed < target_frame) {
                double delay_ms = (target_frame - elapsed) * 1000.0;
                if (delay_ms > 0.0) {
                    SDL_Delay(static_cast<Uint32>(delay_ms));
                }
            }
        }
    }

    ShutdownWorldRenderer(world_renderer);
    DestroyPostProcess(post);
    ShutdownGame(game);
    ShutdownApplication(app);
    return 0;
}
