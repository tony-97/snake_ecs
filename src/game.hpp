#pragma once

#include "game_factory.hpp"
#include "systems.hpp"
#include "types.hpp"

#include <mutex>

struct Game_t : ECS::Uncopyable_t
{
  explicit Game_t()
  {
    g_fact.LoadMainScene();
    ev_man.suscribe<ev::Collision_t>(*this);
    ecs_man.ForEach<e::Collidable_t>([&](auto& phy, auto& col, auto e) {
      auto      pos{ phy.position };
      Rectangle r{ pos.x - col.size, pos.y - col.size, col.size * 1.0f, col.size * 1.0f };
      col.key = g_data.qd_tree.insert(e, r);
    });
  }

  void grow_snake()
  {
    ++g_data.score;
    auto [tpos, tcol, tren]{ ecs_man.GetComponents<c::Physics_t, c::Collider_t, c::Render_t>(g_data.tail) };
    auto ss  = ecs_man.TransformTo<e::SnakeSegment_t>(g_data.tail);
    auto ren = tren;
    --ren.index;
    g_data.tail = g_fact.EntityFromConfig<e::SnakeTail_t>(
      c::SnakeSegment_t{ ecs_man.GetBaseID<e::Collidable_t>(ss) }, c::Physics_t{ tpos.position }, tcol, ren);
    ecs_man.Match<e::Collidable_t>(g_data.tail, [&](auto& phy, auto& col, auto e) {
      auto      pos{ phy.position };
      Rectangle r{ pos.x - col.size, pos.y - col.size, col.size * 1.0f, col.size * 1.0f };
      col.key = g_data.qd_tree.insert(e, r);
    });
    g_fact.ConfigureEntityFromConfig(ss);
    if (g_data.score % 10 == 0) {
      g_fact.grow(g_data.head, 1);
      ecs_man.ParallelForEach<e::SnakeSegment_t>([&](auto e) { g_fact.grow(e, 1); });
    }
  }

  void operator()(ev::Collision_t ev)
  {
    auto [e1, e2]{ ev };
    query_creation([&]() { grow_snake(); });
    query_destruction(e2);
  }

  auto run() -> void
  {
    while (!WindowShouldClose()) {
      DrawingRoutine([&]() {
        ren_sys.update(ecs_man, g_data);
        // dbg_ren_sys.update(ecs_man, g_data);
      });
      inp_sys.update(ecs_man, g_data);
      snake_sys.update(ecs_man);
      col_sys.update(ev_man, ecs_man, g_data);
      process_queue(queue_destruction);
      process_queue(queue_creation);
    }
  }

  auto query_destruction(auto e) -> void
  {
    queue_destruction.emplace_back([ent = e, this]() {
      ecs_man.Match<e::Collidable_t>(ent, [this](auto&, auto& col) { g_data.qd_tree.erase(col.key); });
      ecs_man.Destroy(ent);
    });
  }

  template<class Callaback_t> auto query_creation(Callaback_t&& cb) -> void
  {
    queue_creation.emplace_back(std::forward<Callaback_t>(cb));
  }

  auto process_queue(std::vector<std::function<void()>>& queue) -> void
  {
    for (auto&& fn : queue) {
      fn();
    }
    queue.clear();
  }

private:
  template<class Callaback_t, class... Args_t> auto DrawingRoutine(Callaback_t&& fn, Args_t&&... args) const -> void
  {
    BeginDrawing();
    std::invoke(std::forward<Callaback_t>(fn), std::forward<Args_t>(args)...);
    EndDrawing();
  }

  ECSMan_t            ecs_man{};
  EVMan_t             ev_man{};
  GameData_t          g_data{};
  const GameFactory_t g_fact{ ecs_man, g_data };

  std::vector<std::function<void()>> queue_destruction{};
  std::vector<std::function<void()>> queue_creation{};

  const sys::Render_t      ren_sys{ g_data.screen_width, g_data.screen_height, "Snake 3000", res_man };
  const sys::DebugRender_t dbg_ren_sys{};
  const ResourceManager_t  res_man{};
  const sys::Snake_t       snake_sys{};
  const sys::Input_t       inp_sys{};
  const sys::Collider_t    col_sys{};
};
