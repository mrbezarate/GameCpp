#include "render/WorldRenderer.h"

#include <SDL_opengl.h>

#include "core/Log.h"
#include "math/Vec3.h"
#include "render/Renderer.h"
#include "render/Texture.h"

bool InitWorldRenderer(WorldRenderer& renderer) {
    renderer.ground_tex = LoadTexture2D("../assets/textures/Gress.png");
    if (renderer.ground_tex.id == 0) {
        renderer.ground_tex = LoadTexture2D("../assets/textures/ground.png");
    }
    if (renderer.ground_tex.id == 0) {
        renderer.ground_tex = LoadTexture2D("../assets/textures/graund.png"); // fallback to provided file
    }
    if (renderer.ground_tex.id == 0) {
        renderer.ground_tex = CreateCheckerTexture(128, 16);
    }
    
    renderer.wall_tex = LoadTexture2D("../assets/textures/BrickWall.png");
    if (renderer.wall_tex.id == 0) {
        renderer.wall_tex = CreateCheckerTexture(64, 8);
    }
    
    renderer.road_tex = LoadTexture2D("../assets/textures/Road.png");
    if (renderer.road_tex.id == 0) {
        renderer.road_tex = CreateRoadTexture(256, 256, 8, 28, 18);
    }
    
    renderer.window_tex = LoadTexture2D("../assets/textures/WindowGlass.png");
    if (renderer.window_tex.id == 0) {
        renderer.window_tex = CreateCheckerTexture(32, 4);
    }
    
    renderer.door_tex = LoadTexture2D("../assets/textures/DoorMetal.png");
    if (renderer.door_tex.id == 0) {
        renderer.door_tex = CreateCheckerTexture(32, 4);
    }
    
    renderer.roof_tex = LoadTexture2D("../assets/textures/RoofMetal.png");
    if (renderer.roof_tex.id == 0) {
        renderer.roof_tex = LoadTexture2D("../assets/textures/RoofMetal.jpg"); // fallback to provided file
    }
    if (renderer.roof_tex.id == 0) {
        renderer.roof_tex = CreateCheckerTexture(64, 8);
    }
    
    renderer.concrete_tex = LoadTexture2D("../assets/textures/Concrete.png");
    if (renderer.concrete_tex.id == 0) {
        renderer.concrete_tex = CreateCheckerTexture(64, 8);
    }

    renderer.container_tex = LoadTexture2D("../assets/textures/Container.png");
    if (renderer.container_tex.id == 0) {
        renderer.container_tex = LoadTexture2D("../assets/textures/conrainer.png"); // fallback to provided file
    }
    if (renderer.container_tex.id == 0) {
        renderer.container_tex = CreateCheckerTexture(64, 8);
    }
    
    LogInfo("World renderer initialized");
    return true;
}

void RenderWorld(const WorldRenderer& renderer, const World& world, int ww, int wh, float cam_yaw, float cam_pitch, Vec3 cam_pos, bool debug_colliders) {
    Setup3D(ww, wh, 60.0f, cam_yaw, cam_pitch, cam_pos);
    
    for (const auto& item : world.renderables) {
        switch (item.kind) {
            case RenderKind::Ground:
                DrawBoxTextured(item.box, renderer.ground_tex, 1.0f, 1.0f, 1.0f, 7.0f);
                break;
            case RenderKind::Road:
                {
                    bool rotate_uv = item.box.half.x > item.box.half.z;
                    if (rotate_uv) {
                        glMatrixMode(GL_TEXTURE);
                        glPushMatrix();
                        glLoadIdentity();
                        glRotatef(90.0f, 0.0f, 0.0f, 1.0f);
                        glMatrixMode(GL_MODELVIEW);
                    }
                    DrawBoxTextured(item.box, renderer.road_tex, 1.0f, 1.0f, 1.0f, 5.0f);
                    if (rotate_uv) {
                        glMatrixMode(GL_TEXTURE);
                        glPopMatrix();
                        glMatrixMode(GL_MODELVIEW);
                    }
                }
                break;
            case RenderKind::Sidewalk:
                DrawBoxTextured(item.box, renderer.concrete_tex, 0.9f, 0.9f, 0.9f, 3.5f);
                break;
            case RenderKind::Building:
                DrawBoxTextured(item.box, renderer.wall_tex, 1.0f, 1.0f, 1.0f, 2.8f);
                DrawBuildingNumber(item.box, item.number, item.facing);
                break;
            case RenderKind::Roof:
                DrawBoxTextured(item.box, renderer.roof_tex, 0.7f, 0.7f, 0.75f, 2.0f);
                break;
            case RenderKind::WindowBand:
                DrawBoxTextured(item.box, renderer.window_tex, 0.7f, 0.85f, 1.0f, 1.2f);
                break;
            case RenderKind::Door:
                DrawBoxTextured(item.box, renderer.door_tex, 0.9f, 0.85f, 0.8f, 1.0f);
                break;
            case RenderKind::Wall:
                DrawBoxTextured(item.box, renderer.wall_tex, 0.8f, 0.8f, 0.8f, 4.0f);
                break;
            case RenderKind::Container:
                DrawBoxTextured(item.box, renderer.container_tex, 0.9f, 0.9f, 0.9f, 2.5f);
                break;
        }
    }

    if (debug_colliders) {
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(2.0f);
        glColor4f(0.95f, 0.2f, 0.95f, 0.9f);
        for (const auto& solid : world.solids) {
            DrawBox(solid, 1.0f, 0.2f, 1.0f);
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
        glDisable(GL_BLEND);
    }
}

void ShutdownWorldRenderer(WorldRenderer& renderer) {
    DestroyTexture(renderer.container_tex);
    DestroyTexture(renderer.concrete_tex);
    DestroyTexture(renderer.roof_tex);
    DestroyTexture(renderer.door_tex);
    DestroyTexture(renderer.window_tex);
    DestroyTexture(renderer.road_tex);
    DestroyTexture(renderer.wall_tex);
    DestroyTexture(renderer.ground_tex);
}


