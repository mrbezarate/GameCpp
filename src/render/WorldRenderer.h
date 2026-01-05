#pragma once

#include "render/Texture.h"
#include "world/World.h"

struct WorldRenderer {
    Texture2D ground_tex;
    Texture2D wall_tex;
    Texture2D road_tex;
    Texture2D window_tex;
    Texture2D door_tex;
    Texture2D roof_tex;
    Texture2D concrete_tex;
    Texture2D container_tex;
};

bool InitWorldRenderer(WorldRenderer& renderer);
void RenderWorld(const WorldRenderer& renderer, const World& world, int ww, int wh, float cam_yaw, float cam_pitch, Vec3 cam_pos, bool debug_colliders);
void ShutdownWorldRenderer(WorldRenderer& renderer);


