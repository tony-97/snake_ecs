#pragma once

#include "types.hpp"
#include "systems.hpp"
#include "game_factory.hpp"

#include <mutex>

struct Game_t : ECS::Uncopyable_t
{
    explicit Game_t()
    {
        ev_man.suscribe<ev::Collision_t>(*this);
    }

    void grow_snake()
    {
        ++g_data.score;
        auto [tpos, tcol, tren] { ecs_man.GetComponents<c::Physics_t, c::Collider_t, c::Render_t>(g_data.tail) };
        auto ss = ecs_man.TransformTo<e::SnakeSegment_t>(g_data.tail);
        g_data.tail = ecs_man.CreateEntity<e::SnakeTail_t>(
        c::SnakeSegment_t{ ecs_man.GetBase<e::Collidable_t>(ss) },
        c::Physics_t{ tpos.position },
        tcol,
        tren
        );
        if (g_data.score % 10 == 0) {
            g_fact.grow(g_data.head, 1);
            ecs_man.ParallelForEach<e::SnakeSegment_t>([&](auto e){
                            g_fact.grow(e, 1);
                    });
        }
    }

    void operator()(ev::Collision_t ev)
    {
        auto [e1, e2] { ev };
        query_creation([&]() {
                    grow_snake();
                });
        query_destruction(e2);
    }

    auto run() -> void
    {
        while (!WindowShouldClose()) {
            inp_sys.update(ecs_man, g_data);
            snake_sys.update(ecs_man, g_data);
            col_sys.update(ev_man, ecs_man, g_data);
            DrawingRoutine([&](){
                ren_sys.update(ecs_man, g_data);
            });
            process_queue(queue_destruction);
            process_queue(queue_creation);
        }
    }

    auto query_destruction(auto e) -> void
    {
        queue_destruction.emplace_back([ent=e,this]() {
                    ecs_man.Destroy(ent);
                });
    }

    template<class Callaback_t>
    auto query_creation(Callaback_t&& cb) -> void
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

    template<class Callaback_t, class... Args_t>
    auto DrawingRoutine(Callaback_t&& fn, Args_t&&... args) const -> void
    {
        BeginDrawing();
        std::invoke(std::forward<Callaback_t>(fn), std::forward<Args_t>(args)...);
        EndDrawing();
    }

    ECSMan_t ecs_man {  };
    EVMan_t ev_man {  };
    GameFactory_t g_fact { ecs_man };
    GameData_t g_data { g_fact.LoadMainScene(1366, 768) };

    std::vector<std::function<void()>> queue_destruction {  };
    std::vector<std::function<void()>> queue_creation {  };

    sys::Render_t ren_sys { g_data.screen_width, g_data.screen_height, "Snake 3000", res_man };
    ResourceManager_t  res_man {  };
    sys::Snake_t snake_sys {  };
    sys::Input_t inp_sys {  };
    sys::Collider_t col_sys {  };
};
