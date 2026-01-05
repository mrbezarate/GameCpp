#pragma once

#include <vector>

#include "physics/Collision.h"

enum class RenderKind {
    Ground,
    Road,
    Sidewalk,
    Building,
    Roof,
    WindowBand,
    Door,
    Wall,
    Container
};

enum class Facing {
    PosZ,
    NegZ,
    PosX,
    NegX
};

struct RenderBox {
    AABB box;
    RenderKind kind = RenderKind::Building;
    int number = 0;
    Facing facing = Facing::PosZ;
};

struct World {
    std::vector<AABB> solids;
    std::vector<RenderBox> renderables;
};

World CreateDefaultWorld();
    bool SaveWorld(const World& world, const char* filename);
World LoadWorld(const char* filename);
