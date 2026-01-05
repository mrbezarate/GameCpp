#pragma once

#include <SDL_opengl.h>
#include <vector>

#include "math/Vec3.h"

struct GltfPrimitive {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    int index_count = 0;
    bool has_uv = false;
    GLuint texture_id = 0;
    float base_color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float transform[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    };
};

struct GltfModel {
    std::vector<GltfPrimitive> primitives;
    std::vector<GLuint> textures;
};

bool LoadGltfModel(const char* path, GltfModel& out_model);
void DrawGltfModel(const GltfModel& model, Vec3 pos, float yaw_deg, float scale);
void DestroyGltfModel(GltfModel& model);
