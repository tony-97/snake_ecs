#pragma once

#include <raylib.h>
#include <raymath.h>

#include "entities.hpp"
#include "type_aliases.hpp"
#include "types.hpp"

namespace sys
{

struct Render_t
{
    explicit Render_t(unsigned width, unsigned height, const char* title, const ResourceManager_t& res_man)
        : mResMan { res_man }
    {
        InitWindow(width, height, title);
        SetWindowState(FLAG_FULLSCREEN_MODE);
    }

    auto update(const ECSMan_t& ecs_man, const GameData_t& g_data) const -> void
    {
        ClearBackground(BLACK);

        BeginMode2D(g_data.camera);
        ecs_man.ForEach<e::Renderable_t>([&](auto& ren, auto& pos) {
                    auto dest_rec { ren.crop };
                    dest_rec.width  *= ren.scale; 
                    dest_rec.height *= ren.scale; 
                    dest_rec.x = pos.position.x; dest_rec.y = pos.position.y;
                    auto rot { Vector2Angle(pos.position, pos.direction) };
                    DrawTexturePro(mResMan.mAtlas, ren.crop, dest_rec, { dest_rec.width * 0.5f, dest_rec.height * 0.5f }, rot, ren.color);
                });
        EndMode2D();
        DrawFPS(10, 10);
    }

    ~Render_t()
    {
        CloseWindow();
    }
private:
    const ResourceManager_t& mResMan;
};

struct Snake_t
{
    constexpr explicit Snake_t() = default;

    constexpr auto update(ECSMan_t& ecs_man, const GameData_t& g_data) const -> void
    {
        ecs_man.ParallelForEach<e::SnakeSegment_t>([&](auto&, auto& pos, auto& col, auto& ss) {
            auto [t_pos, t_col] { ecs_man.GetComponents<c::Physics_t, c::Collider_t>(ss.target) };
            auto max_distance { (col.size + t_col.size) * 0.5f };
            if (Vector2Distance(pos.position, t_pos.position) > max_distance) {
                Vector2 dir { Vector2Subtract(pos.position, t_pos.position) };
                dir = Vector2Normalize(dir);
                pos.position = Vector2Add(t_pos.position, Vector2Scale(dir, max_distance));
            } //else if (Vector2Distance(pos.position, t_pos.position) > col.size + t_col.size) {
            //    Vector2 direction = Vector2Subtract(t_pos.position, pos.position);
            //    direction = Vector2Normalize(direction);
            //    pos.position = Vector2Add(pos.position, Vector2Scale(direction, col.size + t_col.size - Vector2Distance(pos.position, t_pos.position)));
            //}
        });
    }
};

struct Input_t
{
    constexpr explicit Input_t() = default;

    auto update(ECSMan_t& ecs_man, GameData_t& g_data) const -> void
    {
        auto& phy { ecs_man.GetComponent<c::Physics_t>(g_data.head) };
        // Get mouse position
        Vector2 mousePosition = GetScreenToWorld2D(GetMousePosition(), g_data.camera);

        // Calculate desired direction
        Vector2 desiredDirection = Vector2Subtract(mousePosition, phy.position);
        desiredDirection = Vector2Normalize(desiredDirection);

        // Gradually turn towards the desired direction
        phy.velocity = Vector2Normalize(phy.velocity);
        phy.velocity = Vector2Lerp(phy.velocity, desiredDirection, 1.0f);

        // Update head position
        float acceleration = 1.0f;
        if (IsKeyDown(KEY_SPACE)) {
            acceleration *= 2.0f;
        }
        phy.position = Vector2Add(phy.position, Vector2Scale(phy.velocity, g_data.speed * acceleration * GetFrameTime()));
        g_data.camera.target = phy.position;
    }
};

struct Collider_t
{
    constexpr explicit Collider_t() = default;

    constexpr auto update(EVMan_t& ev_man, const ECSMan_t& ecs_man, const GameData_t& g_data) const -> void
    {
        ecs_man.ParallelForEach<e::Food_t>([&](auto food) {
            ecs_man.Match<e::Collidable_t>(food,
                [&](auto& food_phy, auto& food_col, auto food_e) {
                    ecs_man.Match<e::Collidable_t>(g_data.head,
                        [&](auto& phy, auto& col, auto e) {
                            if (Vector2Distance(phy.position, food_phy.position) < col.size + food_col.size) {
                                ev_man.publish(ev::Collision_t{ e, food_e });
                            }
                        });
                });
        });
    }
};

} // namespace systems
