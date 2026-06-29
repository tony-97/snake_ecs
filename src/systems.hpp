#pragma once

#include "entities.hpp"
#include "type_aliases.hpp"
#include "types.hpp"
#include "game_factory.hpp"

#include <algorithm>
#include <format>
#include <raylib.h>
#include <raymath.h>

namespace sys {

struct Render_t
{
  explicit Render_t(unsigned width, unsigned height, const char* title, const ResourceManager_t& res_man)
    : mResMan{ res_man }
  {
    InitWindow(width, height, title);
    SetWindowState(FLAG_FULLSCREEN_MODE);
  }

  auto update(const ECSMan_t& ecs_man, const GameData_t& g_data) const -> void
  {
    ClearBackground(BLACK);
    BeginMode2D(g_data.camera);
    auto      camera = g_data.camera;
    float     left   = camera.target.x - (camera.offset.x / camera.zoom);
    float     top    = camera.target.y - (camera.offset.y / camera.zoom);
    Rectangle camera_view{ left, top, GetScreenWidth() * 1.0f, GetScreenHeight() * 1.0f };
    auto      entities{ g_data.qd_tree.search(camera_view) };
    // std::sort(std::execution::par_unseq, entities.begin(), entities.end(), [&](auto k1, auto k2) {
    std::sort(entities.begin(), entities.end(), [&](auto k1, auto k2) {
      auto comp{ false };
      ecs_man.Match<e::Renderable_t>(g_data.qd_tree[k1], [&](auto& ren1, auto&) {
        ecs_man.Match<e::Renderable_t>(g_data.qd_tree[k2], [&](auto& ren2, auto&) { comp = ren1.index < ren2.index; });
      });
      return comp;
    });
    for (auto&& e : entities) {
      ecs_man.Match<e::Renderable_t>(g_data.qd_tree[e], [&](auto& ren, auto& pos) {
        auto dest_rec{ ren.crop };
        dest_rec.width *= ren.scale;
        dest_rec.height *= ren.scale;
        dest_rec.x = pos.position.x;
        dest_rec.y = pos.position.y;
        auto rot{ Vector2Angle(Vector2{}, pos.direction) * RAD2DEG + 90 };
        DrawTexturePro(
          mResMan.mAtlas, ren.crop, dest_rec, { dest_rec.width * 0.5f, dest_rec.height * 0.5f }, rot, ren.color);
      });
    }
    EndMode2D();
    DrawFPS(10, 10);
  }

  ~Render_t() { CloseWindow(); }

private:
  const ResourceManager_t& mResMan;
};

struct DebugRender_t
{
  constexpr explicit DebugRender_t() = default;

  constexpr static void draw_quad_tree(auto& qd_tree, Rectangle area)
  {
    for (auto&& qd_node : qd_tree.quads(area)) {
      DrawRectangleLinesEx(qd_node->mArea, 1.0f, GREEN);
    }
  }

  auto update(const ECSMan_t& ecs_man, const GameData_t& g_data) const -> void
  {
    BeginMode2D(g_data.camera);
    auto      camera = g_data.camera;
    float     left   = camera.target.x - (camera.offset.x / camera.zoom);
    float     top    = camera.target.y - (camera.offset.y / camera.zoom);
    Rectangle camera_view{ left, top, GetScreenWidth() * 1.0f, GetScreenHeight() * 1.0f };
    draw_quad_tree(g_data.qd_tree, camera_view);
    auto entities{ g_data.qd_tree.search(camera_view) };
    for (auto&& e : entities) {
      ecs_man.Match<e::Collidable_t>(g_data.qd_tree[e], [&](auto& pos, auto& col) {
        DrawCircleLines(pos.position.x, pos.position.y, col.size, BLUE);
        DrawLineV(pos.position, Vector2Add(pos.position, Vector2Scale(pos.direction, col.size)), RED);
      });
    }
    EndMode2D();
    DrawFPS(10, 10);
  }
};

struct Snake_t
{
  constexpr explicit Snake_t() = default;

  constexpr auto update(ECSMan_t& ecs_man) const -> void
  {
    ecs_man.ParallelForEach<e::SnakeSegment_t>([&](auto&, auto& pos, auto& col, auto& ss) {
      auto [t_pos, t_col]{ ecs_man.GetComponents<c::Physics_t, c::Collider_t>(ss.target) };
      const auto max_distance{ col.size + t_col.size };
      const auto distance{ Vector2Distance(pos.position, t_pos.position) };
      if (distance > max_distance) {
        Vector2 dir{ Vector2Subtract(pos.position, t_pos.position) };
        pos.direction = dir = Vector2Normalize(dir);
        pos.position        = Vector2Add(t_pos.position, Vector2Scale(dir, max_distance));
      }
    });
  }
};

struct Input_t
{
  constexpr explicit Input_t() = default;

  auto update(ECSMan_t& ecs_man, GameData_t& g_data) const -> void
  {
    auto& phy{ ecs_man.GetComponent<c::Physics_t>(g_data.head) };
    // Get mouse position
    Vector2 mousePosition = GetScreenToWorld2D(GetMousePosition(), g_data.camera);

    // Calculate desired direction
    Vector2 desiredDirection = Vector2Subtract(mousePosition, phy.position);
    desiredDirection         = Vector2Normalize(desiredDirection);
    // Gradually turn towards the desired direction
    phy.velocity  = Vector2Normalize(phy.velocity);
    phy.velocity  = Vector2Lerp(phy.velocity, desiredDirection, 1.0f);
    phy.direction = Vector2Normalize(phy.velocity);

    // Update head position
    float acceleration = 1.0f;
    if (IsKeyDown(KEY_SPACE)) {
      acceleration *= 2.0f;
    }
    phy.position =
      Vector2Add(phy.position,
                 Vector2Scale(phy.velocity, g_data.speed * acceleration * (GetFrameTime() > 1 ? 1 : GetFrameTime())));
    g_data.camera.target = phy.position;
  }
};

struct Collider_t
{
  constexpr explicit Collider_t() = default;

  constexpr auto update(EVMan_t& ev_man, ECSMan_t& ecs_man, const GameData_t& g_data) const -> void
  {
    ecs_man.ForEach<e::move_tag_t>([&](auto e) {
      ecs_man.Match<e::Collidable_t>(e, [&](auto& phy, auto& col) {
        auto      pos{ phy.position };
        Rectangle r{ pos.x - col.size, pos.y - col.size, col.size * 1.0f, col.size * 1.0f };
        g_data.qd_tree.update(r, col.key);
      });
    });
    ecs_man.Match<e::Collidable_t>(g_data.head, [&](auto& phy, auto& col, auto e) {
      auto      pos{ phy.position };
      Rectangle r{ pos.x - col.size, pos.y - col.size, col.size * 1.0f, col.size * 1.0f };
      auto&     qd_tree{ g_data.qd_tree };
      auto      vh{ qd_tree.search(r) };
      for (auto&& h : vh) {
        if (col.id != ecs_man.GetComponent<c::Collider_t>(qd_tree[h]).id) {
          ev_man.publish(ev::Collision_t{ e, qd_tree[h] });
        }
      }
    });
  }
};

struct Gameplay_t
{
  constexpr explicit Gameplay_t() = default;

  constexpr auto update(const EVMan_t& ev_man, CmdMan_t& cmd_man) const -> void
  {
    for (const auto& ev : ev_man.get_events<ev::Collision_t>()) {
      auto [e1, e2]{ ev };
      cmd_man.publish(ev::GrowSnake_t{});
      cmd_man.publish(ev::DestroyEntity_t{ e2 });
    }
  }
};

struct CommandProcessor_t
{
  const GameFactory_t& mGFact;

  explicit CommandProcessor_t(const GameFactory_t& g_fact) : mGFact{ g_fact } {}

  auto update(CmdMan_t& cmd_man, ECSMan_t& ecs_man, GameData_t& g_data) const -> void
  {
    for (const auto& ev : cmd_man.get_events<ev::GrowSnake_t>()) {
      grow_snake(ecs_man, g_data);
    }
    for (const auto& ev : cmd_man.get_events<ev::DestroyEntity_t>()) {
      ecs_man.Match<e::Collidable_t>(ev.entity, [&](auto&, auto& col) {
        g_data.qd_tree.erase(col.key);
      });
      ecs_man.Destroy(ev.entity);
    }
    cmd_man.clear();
  }

private:
  auto grow_snake(ECSMan_t& ecs_man, GameData_t& g_data) const -> void
  {
    ++g_data.score;
    auto [tpos, tcol, tren]{ ecs_man.GetComponents<c::Physics_t, c::Collider_t, c::Render_t>(g_data.tail) };
    auto ss  = ecs_man.TransformTo<e::SnakeSegment_t>(g_data.tail);
    auto ren = tren;
    --ren.index;
    g_data.tail = mGFact.EntityFromConfig<e::SnakeTail_t>(
      c::SnakeSegment_t{ ecs_man.GetBaseID<e::Collidable_t>(ss) }, c::Physics_t{ tpos.position }, tcol, ren);
    ecs_man.Match<e::Collidable_t>(g_data.tail, [&](auto& phy, auto& col, auto e) {
      col.size = tcol.size;
      auto      pos{ phy.position };
      Rectangle r{ pos.x - col.size, pos.y - col.size, col.size * 1.0f, col.size * 1.0f };
      col.key = g_data.qd_tree.insert(e, r);
    });
    mGFact.grow(g_data.head, 1);
    ecs_man.ParallelForEach<e::SnakeSegment_t>([&](auto e) { mGFact.grow(e, 1); });
  }
};

} // namespace systems
