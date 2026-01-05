#pragma once

#include "math/Vec3.h"
#include "physics/Collision.h"
#include "world/World.h"

struct Texture2D;

void Setup3D(int ww, int wh, float fov_deg, float cam_yaw_deg, float cam_pitch_deg, Vec3 cam_pos);
void DrawBox(const AABB& box, float r, float g, float b);
void DrawBoxTextured(const AABB& box, const Texture2D& texture, float tint_r, float tint_g, float tint_b, float tile_size = 1.0f);
void DrawBuildingNumber(const AABB& box, int number, Facing facing);
void BeginUI(int ww, int wh);
void EndUI();
void DrawRect2D(float x, float y, float w, float h, float r, float g, float b, float a);
void DrawText2D(const char* text, float x, float y, float scale, float r, float g, float b, float a);
void DrawCrosshair(int ww, int wh, int size, float r, float g, float b);
