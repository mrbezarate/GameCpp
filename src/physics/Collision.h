#pragma once

#include <vector>

#include "math/Vec3.h"

struct Player;

struct AABB {
    Vec3 center;
    Vec3 half;
};

bool AabbIntersect(const AABB& a, const AABB& b);
bool CheckGrounded(const Player& player, const std::vector<AABB>& world);
void MoveAndCollide(Player& player, const std::vector<AABB>& world, float dt);
