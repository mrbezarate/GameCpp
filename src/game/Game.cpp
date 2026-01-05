#include "game/Game.h"

#include <cmath>

#include "core/Log.h"
#include "physics/Collision.h"

void InitGame(Game& game) {
    game.config = LoadConfig();
    InitClock(game.clock);
    game.player = Player{};
    game.player.pos = Vec3{0.0f, 2.0f, 0.0f};
    game.world = CreateDefaultWorld();
    InitConsole(game.console);
    game.console.max_lines = game.config.max_console_lines;
    game.chat.max_lines = game.config.max_chat_lines;
    game.chat.fade_start = game.config.chat_fade_start;
    game.chat.fade_end = game.config.chat_fade_end;
    game.show_colliders = false;
    
    LogInfo("Game initialized");
}

void UpdateGame(Game& game, const InputState& input, float cam_yaw, float cam_pitch, double dt) {
    (void)cam_pitch; (void)dt;
    
    game.time_sec += dt;
    
    const double fixed_dt = 1.0 / 60.0;
    while (ConsumeFixedStep(game.clock, fixed_dt)) {
        if (!game.noclip) {
            UpdatePlayer(game.player, input, cam_yaw, static_cast<float>(fixed_dt), game.world);
        } else {
            // Noclip movement
            float yaw_rad = -cam_yaw * 0.017453292f;
            Vec3 forward{std::sin(yaw_rad), 0.0f, -std::cos(yaw_rad)};
            Vec3 right{std::cos(yaw_rad), 0.0f, std::sin(yaw_rad)};
            Vec3 up{0.0f, 1.0f, 0.0f};
            
            float speed = game.config.walk_speed;
            if (input.sprint) {
                speed = game.config.sprint_speed * 2.0f; // Faster in noclip
            }
            
            Vec3 move = forward * input.move_forward + right * input.move_right;
            if (input.jump) {
                move = move + up;
            }
            if (input.crouch) {
                move = move - up;
            }
            
            float move_len = std::sqrt(move.x * move.x + move.y * move.y + move.z * move.z);
            if (move_len > 0.0001f) {
                move.x /= move_len;
                move.y /= move_len;
                move.z /= move_len;
            }
            
            game.player.pos.x += move.x * speed * static_cast<float>(fixed_dt);
            game.player.pos.y += move.y * speed * static_cast<float>(fixed_dt);
            game.player.pos.z += move.z * speed * static_cast<float>(fixed_dt);
        }
    }
}

void ShutdownGame(Game& game) {
    (void)game;
    LogInfo("Game shutdown");
}

