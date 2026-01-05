#pragma once

#include "core/Config.h"
#include "core/Time.h"
#include "game/Chat.h"
#include "game/Console.h"
#include "game/Player.h"
#include "input/Input.h"
#include "world/World.h"

struct Game {
    Player player;
    World world;
    Console console;
    Chat chat;
    FixedTimestepClock clock;
    Config config;
    double time_sec = 0.0;
    bool noclip = false;
    bool show_fps = false;
    bool show_colliders = false;
    float fps = 0.0f;
};

void InitGame(Game& game);
void UpdateGame(Game& game, const InputState& input, float cam_yaw, float cam_pitch, double dt);
void ShutdownGame(Game& game);


