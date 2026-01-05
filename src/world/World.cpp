#include "world/World.h"

#include <fstream>
#include <iostream>

namespace {

// ================= НАСТРОЙКИ ГЕНЕРАЦИИ =================
// Установите true, чтобы убрать лишние детали (окна, крыши)
// Это повысит производительность и снизит визуальный шум ("низкое качество")
static const bool SIMPLIFIED_RENDER = true; 
// =======================================================

static void AddSolid(World& world, const AABB& box, RenderKind kind, int number = 0, Facing facing = Facing::PosZ) {
    world.solids.push_back(box);
    world.renderables.push_back(RenderBox{box, kind, number, facing});
}

static void AddRender(World& world, const AABB& box, RenderKind kind, int number = 0, Facing facing = Facing::PosZ) {
    world.renderables.push_back(RenderBox{box, kind, number, facing});
}

static void AddBuilding(World& world, const AABB& box, int number, Facing facing) {
    // Основная коробка здания (физическая + визуальная)
    AddSolid(world, box, RenderKind::Building, number, facing);

    float base_y = box.center.y - box.half.y;
    const float door_height = 4.0f;

    // --- УПРОЩЕНИЕ ГРАФИКИ ---
    // Если включен SIMPLIFIED_RENDER, мы пропускаем крыши и окна.
    if (!SIMPLIFIED_RENDER) {
        // 1. Крыша
        AABB roof = box;
        roof.half.y = 0.7f;
        roof.center.y = box.center.y + box.half.y - roof.half.y;
        AddRender(world, roof, RenderKind::Roof, 0, facing);

        // 2. Окна (Window Band)
        AABB front = box;
        const float door_gap = 0.4f;
        const float window_band_h = 1.8f;
        front.half.y = window_band_h * 0.5f;
        float window_min_y = base_y + door_height + door_gap;
        float window_center_y = window_min_y + front.half.y;
        float top_limit = box.center.y + box.half.y - 0.5f;
        
        if (window_center_y + front.half.y > top_limit) {
            window_center_y = top_limit - front.half.y;
        }
        front.center.y = window_center_y;
        
        if (facing == Facing::PosZ || facing == Facing::NegZ) {
            front.half.x = box.half.x * 0.85f;
            front.half.z = 0.18f;
            float sign = (facing == Facing::PosZ) ? 1.0f : -1.0f;
            front.center.z = box.center.z + sign * (box.half.z + front.half.z);
        } else {
            front.half.z = box.half.z * 0.85f;
            front.half.x = 0.18f;
            float sign = (facing == Facing::PosX) ? 1.0f : -1.0f;   
            front.center.x = box.center.x + sign * (box.half.x + front.half.x);
        }
        AddRender(world, front, RenderKind::WindowBand, 0, facing);
    }

    // 3. Дверь (оставляем, так как она важна для геймплея/понимания масштаба)
    AABB door{};
    float door_half_y = door_height * 0.5f; 
    if (facing == Facing::PosZ || facing == Facing::NegZ) {
        door.half = Vec3{0.7f, door_half_y, 0.2f};
        float sign = (facing == Facing::PosZ) ? 1.0f : -1.0f;
        door.center = Vec3{
            box.center.x - box.half.x * 0.2f,
            base_y + door.half.y,
            box.center.z + sign * (box.half.z + door.half.z)
        };
    } else {
        door.half = Vec3{0.2f, door_half_y, 0.7f};
        float sign = (facing == Facing::PosX) ? 1.0f : -1.0f;
        door.center = Vec3{
            box.center.x + sign * (box.half.x + door.half.x),
            base_y + door.half.y,
            box.center.z + box.half.z * 0.2f
        };
    }
    AddRender(world, door, RenderKind::Door, 0, facing);
}

static void AddContainer(World& world, const AABB& box) {
    AddSolid(world, box, RenderKind::Container);
}

static AABB MakeFacingX(float front_x, float center_z, float half_x, float half_y, float half_z, Facing facing) {
    AABB box{};
    box.half = Vec3{half_x, half_y, half_z};
    box.center.y = half_y;
    box.center.z = center_z;
    box.center.x = (facing == Facing::PosX) ? (front_x - half_x) : (front_x + half_x);
    return box;
}

static AABB MakeFacingZ(float center_x, float front_z, float half_x, float half_y, float half_z, Facing facing) {
    AABB box{};
    box.half = Vec3{half_x, half_y, half_z};
    box.center.y = half_y;
    box.center.x = center_x;
    box.center.z = (facing == Facing::PosZ) ? (front_z - half_z) : (front_z + half_z);
    return box;
}

static void AddStripZ(World& world, float center_x, float half_x, float z0, float z1, float center_y, float half_y, RenderKind kind) {
    if (z1 <= z0) return;
    AABB box{};
    box.center = Vec3{center_x, center_y, (z0 + z1) * 0.5f};
    box.half = Vec3{half_x, half_y, (z1 - z0) * 0.5f};
    AddRender(world, box, kind);
}

static void AddStripX(World& world, float center_z, float half_z, float x0, float x1, float center_y, float half_y, RenderKind kind) {
    if (x1 <= x0) return;
    AABB box{};
    box.center = Vec3{(x0 + x1) * 0.5f, center_y, center_z};
    box.half = Vec3{(x1 - x0) * 0.5f, half_y, half_z};
    AddRender(world, box, kind);
}

static void AddSidewalkCorner(World& world, float center_x, float center_z, float half) {
    AABB box{};
    box.center = Vec3{center_x, 0.03f, center_z};
    box.half = Vec3{half, 0.03f, half};
    AddRender(world, box, RenderKind::Sidewalk);
}

}  // namespace

World CreateDefaultWorld() {
    World world;

    // Ground foundation.
    AABB ground{{0.0f, -2.0f, 0.0f}, {170.0f, 2.0f, 170.0f}};
    AddSolid(world, ground, RenderKind::Ground);

    // Perimeter walls.
    const float wall_h = 8.0f;
    const float wall_t = 1.0f;
    AddSolid(world, {{0.0f, wall_h, -170.0f}, {170.0f, wall_h, wall_t}}, RenderKind::Wall);
    AddSolid(world, {{0.0f, wall_h, 170.0f}, {170.0f, wall_h, wall_t}}, RenderKind::Wall);
    AddSolid(world, {{-170.0f, wall_h, 0.0f}, {wall_t, wall_h, 170.0f}}, RenderKind::Wall);
    AddSolid(world, {{170.0f, wall_h, 0.0f}, {wall_t, wall_h, 170.0f}}, RenderKind::Wall);

    const float road_half = 8.0f;
    const float road_len = 140.0f;
    const float cross_len = 120.0f;
    const float cross_a_z = 40.0f;
    const float cross_b_z = -70.0f;
    const float sidewalk_half = 3.0f;
    const float sidewalk_offset = road_half + sidewalk_half;
    const float gap = 3.0f;
    const float front_main = road_half + sidewalk_half * 2.0f + gap;
    const float cut_half = sidewalk_offset + sidewalk_half;
    const float main_corridor_half = road_half + sidewalk_half * 2.0f;

    // Main road and avenues logic remains same...
    AddRender(world, {{0.0f, 0.02f, 0.0f}, {road_half, 0.02f, road_len}}, RenderKind::Road);
    AddStripX(world, cross_a_z, road_half, -cross_len, -road_half, 0.02f, 0.02f, RenderKind::Road);
    AddStripX(world, cross_a_z, road_half, road_half, cross_len, 0.02f, 0.02f, RenderKind::Road);
    AddStripX(world, cross_b_z, road_half, -cross_len, -road_half, 0.02f, 0.02f, RenderKind::Road);
    AddStripX(world, cross_b_z, road_half, road_half, cross_len, 0.02f, 0.02f, RenderKind::Road);

    // Sidewalks
    AddStripZ(world, -sidewalk_offset, sidewalk_half, -road_len, cross_b_z - cut_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripZ(world, -sidewalk_offset, sidewalk_half, cross_b_z + cut_half, cross_a_z - cut_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripZ(world, -sidewalk_offset, sidewalk_half, cross_a_z + cut_half, road_len, 0.03f, 0.03f, RenderKind::Sidewalk);

    AddStripZ(world, sidewalk_offset, sidewalk_half, -road_len, cross_b_z - cut_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripZ(world, sidewalk_offset, sidewalk_half, cross_b_z + cut_half, cross_a_z - cut_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripZ(world, sidewalk_offset, sidewalk_half, cross_a_z + cut_half, road_len, 0.03f, 0.03f, RenderKind::Sidewalk);

    AddStripX(world, cross_a_z - sidewalk_offset, sidewalk_half, -cross_len, -main_corridor_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripX(world, cross_a_z - sidewalk_offset, sidewalk_half, main_corridor_half, cross_len, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripX(world, cross_a_z + sidewalk_offset, sidewalk_half, -cross_len, -main_corridor_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripX(world, cross_a_z + sidewalk_offset, sidewalk_half, main_corridor_half, cross_len, 0.03f, 0.03f, RenderKind::Sidewalk);

    AddStripX(world, cross_b_z - sidewalk_offset, sidewalk_half, -cross_len, -main_corridor_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripX(world, cross_b_z - sidewalk_offset, sidewalk_half, main_corridor_half, cross_len, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripX(world, cross_b_z + sidewalk_offset, sidewalk_half, -cross_len, -main_corridor_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripX(world, cross_b_z + sidewalk_offset, sidewalk_half, main_corridor_half, cross_len, 0.03f, 0.03f, RenderKind::Sidewalk);

    const float corner_x = road_half + sidewalk_half;
    const float corner_off = road_half + sidewalk_half;
    AddSidewalkCorner(world, corner_x, cross_a_z + corner_off, sidewalk_half);
    AddSidewalkCorner(world, -corner_x, cross_a_z + corner_off, sidewalk_half);
    AddSidewalkCorner(world, corner_x, cross_a_z - corner_off, sidewalk_half);
    AddSidewalkCorner(world, -corner_x, cross_a_z - corner_off, sidewalk_half);
    AddSidewalkCorner(world, corner_x, cross_b_z + corner_off, sidewalk_half);
    AddSidewalkCorner(world, -corner_x, cross_b_z + corner_off, sidewalk_half);
    AddSidewalkCorner(world, corner_x, cross_b_z - corner_off, sidewalk_half);
    AddSidewalkCorner(world, -corner_x, cross_b_z - corner_off, sidewalk_half);

    // East service road
    const float service_x = 70.0f;
    const float service_half = 4.5f;
    const float service_len = 120.0f;
    const float service_sidewalk = 2.5f;
    const float service_gap = 2.5f;
    const float service_front = service_half + service_sidewalk * 2.0f + service_gap;
    AddStripZ(world, service_x, service_half, -service_len, cross_b_z - cut_half, 0.02f, 0.02f, RenderKind::Road);
    AddStripZ(world, service_x, service_half, cross_b_z + cut_half, cross_a_z - cut_half, 0.02f, 0.02f, RenderKind::Road);
    AddStripZ(world, service_x, service_half, cross_a_z + cut_half, service_len, 0.02f, 0.02f, RenderKind::Road);

    float service_sidewalk_x = service_x - (service_half + service_sidewalk);
    AddStripZ(world, service_sidewalk_x, service_sidewalk, -service_len, cross_b_z - cut_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripZ(world, service_sidewalk_x, service_sidewalk, cross_b_z + cut_half, cross_a_z - cut_half, 0.03f, 0.03f, RenderKind::Sidewalk);
    AddStripZ(world, service_sidewalk_x, service_sidewalk, cross_a_z + cut_half, service_len, 0.03f, 0.03f, RenderKind::Sidewalk);

    // =========================================================================================
    // ЗДАНИЯ С АВТОМАТИЧЕСКОЙ НУМЕРАЦИЕЙ
    // =========================================================================================
    
    // Buildings along main road (fronts aligned to sidewalks).
    const float front_w = -front_main;
    const float front_e = front_main;
    
    // Группа 100 (Западная сторона главной дороги)
    int id_gen = 100;
    AddBuilding(world, MakeFacingX(front_w, -115.0f, 13.0f, 10.0f, 22.0f, Facing::PosX), ++id_gen, Facing::PosX);
    AddBuilding(world, MakeFacingX(front_w, -30.0f, 11.0f, 8.0f, 16.0f, Facing::PosX), ++id_gen, Facing::PosX);
    AddBuilding(world, MakeFacingX(front_w, 10.0f, 9.0f, 7.0f, 12.0f, Facing::PosX), ++id_gen, Facing::PosX);
    AddBuilding(world, MakeFacingX(front_w, 85.0f, 14.0f, 10.0f, 20.0f, Facing::PosX), ++id_gen, Facing::PosX);
    AddBuilding(world, MakeFacingX(front_w, 125.0f, 10.0f, 8.0f, 14.0f, Facing::PosX), ++id_gen, Facing::PosX);

    // Группа 200 (Восточная сторона главной дороги)
    id_gen = 200;
    AddBuilding(world, MakeFacingX(front_e, -110.0f, 14.0f, 10.0f, 20.0f, Facing::NegX), ++id_gen, Facing::NegX);
    AddBuilding(world, MakeFacingX(front_e, -25.0f, 11.0f, 8.0f, 15.0f, Facing::NegX), ++id_gen, Facing::NegX);
    AddBuilding(world, MakeFacingX(front_e, 5.0f, 9.0f, 7.0f, 11.0f, Facing::NegX), ++id_gen, Facing::NegX);
    AddBuilding(world, MakeFacingX(front_e, 80.0f, 13.0f, 9.0f, 18.0f, Facing::NegX), ++id_gen, Facing::NegX);
    AddBuilding(world, MakeFacingX(front_e, 120.0f, 10.0f, 8.0f, 14.0f, Facing::NegX), ++id_gen, Facing::NegX);

    // Cross-road street blocks (shops + mid-rise fronts).
    const float cross_front_s_a = cross_a_z - front_main;
    const float cross_front_n_a = cross_a_z + front_main;
    
    // Группа 300 (Перекресток А)
    id_gen = 300;
    AddBuilding(world, MakeFacingZ(-95.0f, cross_front_s_a, 18.0f, 5.0f, 9.0f, Facing::PosZ), ++id_gen, Facing::PosZ);
    AddBuilding(world, MakeFacingZ(-55.0f, cross_front_s_a, 14.0f, 6.0f, 8.0f, Facing::PosZ), ++id_gen, Facing::PosZ);
    AddBuilding(world, MakeFacingZ(45.0f, cross_front_s_a, 12.0f, 5.0f, 8.0f, Facing::PosZ), ++id_gen, Facing::PosZ);
    AddBuilding(world, MakeFacingZ(100.0f, cross_front_s_a, 16.0f, 6.0f, 9.0f, Facing::PosZ), ++id_gen, Facing::PosZ);

    // Группа 310 (Северная сторона Перекрестка А)
    id_gen = 310;
    AddBuilding(world, MakeFacingZ(-90.0f, cross_front_n_a, 16.0f, 6.0f, 10.0f, Facing::NegZ), ++id_gen, Facing::NegZ);
    AddBuilding(world, MakeFacingZ(-50.0f, cross_front_n_a, 12.0f, 5.0f, 8.0f, Facing::NegZ), ++id_gen, Facing::NegZ);
    AddBuilding(world, MakeFacingZ(45.0f, cross_front_n_a, 12.0f, 5.0f, 8.0f, Facing::NegZ), ++id_gen, Facing::NegZ);
    AddBuilding(world, MakeFacingZ(95.0f, cross_front_n_a, 16.0f, 6.0f, 10.0f, Facing::NegZ), ++id_gen, Facing::NegZ);

    const float cross_front_s_b = cross_b_z - front_main;
    const float cross_front_n_b = cross_b_z + front_main;
    
    // Группа 400 (Перекресток B)
    id_gen = 400;
    AddBuilding(world, MakeFacingZ(-95.0f, cross_front_s_b, 18.0f, 5.0f, 10.0f, Facing::PosZ), ++id_gen, Facing::PosZ);
    AddBuilding(world, MakeFacingZ(-55.0f, cross_front_s_b, 14.0f, 5.0f, 9.0f, Facing::PosZ), ++id_gen, Facing::PosZ);
    AddBuilding(world, MakeFacingZ(45.0f, cross_front_s_b, 12.0f, 5.0f, 9.0f, Facing::PosZ), ++id_gen, Facing::PosZ);
    AddBuilding(world, MakeFacingZ(100.0f, cross_front_s_b, 16.0f, 5.0f, 10.0f, Facing::PosZ), ++id_gen, Facing::PosZ);

    // Группа 410 (Северная сторона Перекрестка B)
    id_gen = 410;
    AddBuilding(world, MakeFacingZ(-90.0f, cross_front_n_b, 16.0f, 5.0f, 9.0f, Facing::NegZ), ++id_gen, Facing::NegZ);
    AddBuilding(world, MakeFacingZ(-50.0f, cross_front_n_b, 12.0f, 5.0f, 8.0f, Facing::NegZ), ++id_gen, Facing::NegZ);
    AddBuilding(world, MakeFacingZ(45.0f, cross_front_n_b, 12.0f, 5.0f, 8.0f, Facing::NegZ), ++id_gen, Facing::NegZ);
    AddBuilding(world, MakeFacingZ(95.0f, cross_front_n_b, 16.0f, 5.0f, 9.0f, Facing::NegZ), ++id_gen, Facing::NegZ);

    // Industrial strip along service road.
    const float service_front_e = service_x + service_front;
    // Группа 500 (Индустриальная зона)
    id_gen = 500;
    AddBuilding(world, MakeFacingX(service_front_e, -120.0f, 18.0f, 7.0f, 18.0f, Facing::NegX), ++id_gen, Facing::NegX);
    AddBuilding(world, MakeFacingX(service_front_e, -10.0f, 20.0f, 8.0f, 22.0f, Facing::NegX), ++id_gen, Facing::NegX);
    AddBuilding(world, MakeFacingX(service_front_e, 110.0f, 18.0f, 7.0f, 18.0f, Facing::NegX), ++id_gen, Facing::NegX);

    // Yard containers / storage stacks.
    AddContainer(world, {{130.0f, 3.0f, -40.0f}, {6.0f, 3.0f, 6.0f}});
    AddContainer(world, {{130.0f, 3.0f, -20.0f}, {6.0f, 3.0f, 6.0f}});
    AddContainer(world, {{130.0f, 3.0f, 0.0f}, {6.0f, 3.0f, 6.0f}});
    AddContainer(world, {{130.0f, 3.0f, 20.0f}, {6.0f, 3.0f, 6.0f}});
    AddContainer(world, {{115.0f, 3.0f, -10.0f}, {6.0f, 3.0f, 6.0f}});

    return world;
}

bool SaveWorld(const World& world, const char* filename) {
    std::ofstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for saving: " << filename << std::endl;
        return false;
    }

    // Save solids
    size_t solids_size = world.solids.size();
    file.write(reinterpret_cast<const char*>(&solids_size), sizeof(solids_size));
    for (const auto& solid : world.solids) {
        file.write(reinterpret_cast<const char*>(&solid), sizeof(AABB));
    }

    // Save renderables
    size_t renderables_size = world.renderables.size();
    file.write(reinterpret_cast<const char*>(&renderables_size), sizeof(renderables_size));
    for (const auto& renderable : world.renderables) {
        file.write(reinterpret_cast<const char*>(&renderable), sizeof(RenderBox));
    }

    return true;
}

World LoadWorld(const char* filename) {
    World world;
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open file for loading: " << filename << std::endl;
        return CreateDefaultWorld(); // Fallback to default
    }

    // Load solids
    size_t solids_size;
    file.read(reinterpret_cast<char*>(&solids_size), sizeof(solids_size));
    world.solids.resize(solids_size);
    for (size_t i = 0; i < solids_size; ++i) {
        file.read(reinterpret_cast<char*>(&world.solids[i]), sizeof(AABB));
    }

    // Load renderables
    size_t renderables_size;
    file.read(reinterpret_cast<char*>(&renderables_size), sizeof(renderables_size));
    world.renderables.resize(renderables_size);
    for (size_t i = 0; i < renderables_size; ++i) {
        file.read(reinterpret_cast<char*>(&world.renderables[i]), sizeof(RenderBox));
    }

    return world;
}