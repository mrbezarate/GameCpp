#include "physics/Collision.h"

#include <cmath>

#include "game/Player.h"

static void ResolveAxis(Player& player, const std::vector<AABB>& world, int axis, Vec3 prev_pos) {
    Vec3 test_pos = player.pos;
    if (axis == 0) {
        test_pos.y = prev_pos.y;
        test_pos.z = prev_pos.z;
    } else if (axis == 1) {
        test_pos.x = prev_pos.x;
        test_pos.z = prev_pos.z;
    } else {
        test_pos.x = prev_pos.x;
        test_pos.y = prev_pos.y;
    }

    float move = 0.0f;
    if (axis == 0) {
        move = test_pos.x - prev_pos.x;
    } else if (axis == 1) {
        move = test_pos.y - prev_pos.y;
    } else {
        move = test_pos.z - prev_pos.z;
    }

    AABB box{test_pos, player.half};
    AABB prev{prev_pos, player.half};
    for (const auto& wall : world) {
        if (!AabbIntersect(box, wall)) {
            continue;
        }
        if (AabbIntersect(prev, wall) && move == 0.0f) {
            continue;
        }

        if (axis == 0) { // x
            if (move > 0.0f) {
                player.pos.x = wall.center.x - wall.half.x - player.half.x;
            } else if (move < 0.0f) {
                player.pos.x = wall.center.x + wall.half.x + player.half.x;
            } else {
                if (prev_pos.x < wall.center.x) {
                    player.pos.x = wall.center.x - wall.half.x - player.half.x;
                } else {
                    player.pos.x = wall.center.x + wall.half.x + player.half.x;
                }
            }
            player.vel.x = 0.0f;
            test_pos.x = player.pos.x;
        } else if (axis == 1) { // y
            if (move > 0.0f) {
                player.pos.y = wall.center.y - wall.half.y - player.half.y;
            } else {
                player.pos.y = wall.center.y + wall.half.y + player.half.y;
                player.grounded = true;
            }
            player.vel.y = 0.0f;
            test_pos.y = player.pos.y;
        } else { // z
            if (move > 0.0f) {
                player.pos.z = wall.center.z - wall.half.z - player.half.z;
            } else if (move < 0.0f) {
                player.pos.z = wall.center.z + wall.half.z + player.half.z;
            } else {
                if (prev_pos.z < wall.center.z) {
                    player.pos.z = wall.center.z - wall.half.z - player.half.z;
                } else {
                    player.pos.z = wall.center.z + wall.half.z + player.half.z;
                }
            }
            player.vel.z = 0.0f;
            test_pos.z = player.pos.z;
        }
        box.center = test_pos;
    }
}

bool AabbIntersect(const AABB& a, const AABB& b) {
    return (std::abs(a.center.x - b.center.x) < (a.half.x + b.half.x)) &&
           (std::abs(a.center.y - b.center.y) < (a.half.y + b.half.y)) &&
           (std::abs(a.center.z - b.center.z) < (a.half.z + b.half.z));
}

bool CheckGrounded(const Player& player, const std::vector<AABB>& world) {
    const float epsilon = 0.02f;
    float bottom = player.pos.y - player.half.y;
    for (const auto& wall : world) {
        float top = wall.center.y + wall.half.y;
        if (bottom < top - epsilon || bottom > top + epsilon) {
            continue;
        }
        if (std::abs(player.pos.x - wall.center.x) < (player.half.x + wall.half.x) &&
            std::abs(player.pos.z - wall.center.z) < (player.half.z + wall.half.z)) {
            return true;
        }
    }
    return false;
}

void MoveAndCollide(Player& player, const std::vector<AABB>& world, float dt) {
    player.grounded = false;
    Vec3 prev = player.pos;

    player.pos.x += player.vel.x * dt;
    ResolveAxis(player, world, 0, prev);
    prev = player.pos;

    player.pos.y += player.vel.y * dt;
    ResolveAxis(player, world, 1, prev);
    prev = player.pos;

    player.pos.z += player.vel.z * dt;
    ResolveAxis(player, world, 2, prev);

    if (!player.grounded) {
        player.grounded = CheckGrounded(player, world);
    }
}
