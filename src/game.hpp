#pragma once

#include "game_factory.hpp"
#include "systems.hpp"
#include "types.hpp"

struct Game_t : ECS::Uncopyable_t
{
  explicit Game_t()
  {
    g_fact.LoadMainScene();
    ecs_man.ForEach<e::Collidable_t>([&](auto& phy, auto& col, auto e) {
      auto      pos{ phy.position };
      Rectangle r{ pos.x - col.size, pos.y - col.size, col.size * 1.0f, col.size * 1.0f };
      col.key = g_data.qd_tree.insert(e, r);
    });
  }

  auto run() -> void
  {
    while (!WindowShouldClose()) {
      DrawingRoutine([&]() {
        ren_sys.update(ecs_man, g_data);
        dbg_ren_sys.update(ecs_man, g_data);
      });
      inp_sys.update(ecs_man, g_data);
      snake_sys.update(ecs_man);
      col_sys.update(ev_man, ecs_man, g_data);
      gameplay_sys.update(ev_man, cmd_man);
      ev_man.clear();
      cmd_sys.update(cmd_man, ecs_man, g_data);
    }
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
  CmdMan_t            cmd_man{};
  GameData_t          g_data{};
  const GameFactory_t g_fact{ ecs_man, g_data };

  const sys::Render_t           ren_sys{ g_data.screen_width, g_data.screen_height, "Snake 3000", res_man };
  const ResourceManager_t       res_man{};
  const sys::DebugRender_t      dbg_ren_sys{};
  const sys::Snake_t            snake_sys{};
  const sys::Input_t            inp_sys{};
  const sys::Collider_t         col_sys{};
  const sys::CommandProcessor_t cmd_sys{ g_fact };
  const sys::Gameplay_t         gameplay_sys{};
};
