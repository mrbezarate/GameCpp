#include "game/Player.h"

#include <cmath>

#include "input/Input.h"
#include "physics/Collision.h"
#include "world/World.h"

static void ApplyFriction(Player& player, float dt, float friction) {
    float speed = std::sqrt(player.vel.x * player.vel.x + player.vel.z * player.vel.z);
    if (speed < 0.001f) {
        player.vel.x = 0.0f;
        player.vel.z = 0.0f;
        return;
    }
    float drop = speed * friction * dt;
    float new_speed = speed - drop;
    if (new_speed < 0.0f) {
        new_speed = 0.0f;
    }
    float scale = new_speed / speed;
    player.vel.x *= scale;
    player.vel.z *= scale;
}

static void Accelerate(Player& player, const Vec3& wish_dir, float wish_speed, float accel, float dt) {
    if (wish_speed <= 0.0f) {
        return;
    }
    float current_speed = player.vel.x * wish_dir.x + player.vel.z * wish_dir.z;
    float add_speed = wish_speed - current_speed;
    if (add_speed <= 0.0f) {
        return;
    }
    float accel_speed = accel * dt * wish_speed;
    if (accel_speed > add_speed) {
        accel_speed = add_speed;
    }
    player.vel.x += wish_dir.x * accel_speed;
    player.vel.z += wish_dir.z * accel_speed;
}

static void AirControl(Player& player, const Vec3& wish_dir, float dt, float control) {
    float speed = std::sqrt(player.vel.x * player.vel.x + player.vel.z * player.vel.z);
    if (speed < 0.001f) {
        return;
    }
    float dot = (player.vel.x * wish_dir.x + player.vel.z * wish_dir.z) / speed;
    if (dot <= 0.0f) {
        return;
    }
    float k = control * dot * dot * dt;
    Vec3 vel_dir{player.vel.x / speed, 0.0f, player.vel.z / speed};
    vel_dir.x = vel_dir.x * (1.0f - k) + wish_dir.x * k;
    vel_dir.z = vel_dir.z * (1.0f - k) + wish_dir.z * k;
    float new_len = std::sqrt(vel_dir.x * vel_dir.x + vel_dir.z * vel_dir.z);
    if (new_len < 0.001f) {
        return;
    }
    vel_dir.x /= new_len;
    vel_dir.z /= new_len;
    player.vel.x = vel_dir.x * speed;
    player.vel.z = vel_dir.z * speed;
}

void UpdatePlayer(Player& player, const InputState& input, float cam_yaw_deg, float dt, const World& world) {
    const float walk_speed = 8.0f;
    const float sprint_speed = 12.5f;
    const float crouch_speed = 4.0f;
    const float ground_accel = 55.0f;
    const float air_accel = 16.0f;
    const float ground_friction = 9.0f;
    const float air_speed_cap = 28.0f;
    const float strafe_boost = 1.25f;
    const float air_control = 8.0f;
    const float gravity = 28.0f;
    const float jump_vel = 11.0f;
    const float stand_half_y = 1.8f;
    const float crouch_half_y = 1.3f;

    float bottom = player.pos.y - player.half.y;
    bool wants_crouch = input.crouch;
    if (!wants_crouch) {
        AABB stand_box{
            {player.pos.x, bottom + stand_half_y, player.pos.z},
            {player.half.x, stand_half_y, player.half.z}
        };
        bool blocked = false;
        for (const auto& wall : world.solids) {
            if (AabbIntersect(stand_box, wall)) {
                blocked = true;
                break;
            }
        }
        if (blocked) {
            wants_crouch = true;
        }
    }

    float target_half_y = wants_crouch ? crouch_half_y : stand_half_y;
    if (std::abs(player.half.y - target_half_y) > 0.001f) {
        player.half.y = target_half_y;
        player.pos.y = bottom + player.half.y;
    }

    float yaw_rad = -cam_yaw_deg * 0.017453292f;
    Vec3 forward{std::sin(yaw_rad), 0.0f, -std::cos(yaw_rad)};
    Vec3 right{std::cos(yaw_rad), 0.0f, std::sin(yaw_rad)};

    Vec3 wish = forward * input.move_forward + right * input.move_right;
    float max_speed = walk_speed;
    if (wants_crouch) {
        max_speed = crouch_speed;
    } else if (input.sprint) {
        max_speed = sprint_speed;
    }

    float wish_len = std::sqrt(wish.x * wish.x + wish.z * wish.z);
    Vec3 wish_dir{};
    if (wish_len > 0.0001f) {
        wish_dir.x = wish.x / wish_len;
        wish_dir.z = wish.z / wish_len;
    }
    bool strafe_only = std::abs(input.move_forward) < 0.001f && std::abs(input.move_right) > 0.001f;
    float wish_speed = (wish_len > 0.0001f) ? max_speed : 0.0f;
    if (!player.grounded && strafe_only) {
        wish_speed = max_speed * strafe_boost;
    }

    bool jumping = input.jump && player.grounded;
    if (player.grounded) {
        if (!jumping) {
            ApplyFriction(player, dt, ground_friction);
        }
        Accelerate(player, wish_dir, wish_speed, ground_accel, dt);
    } else {
        if (wish_speed > air_speed_cap) {
            wish_speed = air_speed_cap;
        }
        Accelerate(player, wish_dir, wish_speed, air_accel, dt);
        AirControl(player, wish_dir, dt, air_control);
    }

    player.vel.y -= gravity * dt;
    if (jumping) {
        player.vel.y = jump_vel;
        player.grounded = false;
    }

    MoveAndCollide(player, world.solids, dt);
}

Vec3 GetCameraPos(const Player& player) {
    return player.pos + Vec3{0.0f, player.half.y * 0.8f, 0.0f};
}
