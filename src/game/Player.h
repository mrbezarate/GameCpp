#pragma once

#include "math/Vec3.h"

struct InputState;
struct World;

struct Player {
    Vec3 pos{0.0f, 2.0f, 0.0f};
    Vec3 vel{0.0f, 0.0f, 0.0f};
    Vec3 half{0.4f, 1.8f, 0.4f};
    bool grounded = false;
};

void UpdatePlayer(Player& player, const InputState& input, float cam_yaw_deg, float dt, const World& world);
Vec3 GetCameraPos(const Player& player);
