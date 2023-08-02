#include "type_aliases.hpp"
#include <raylib.h>

#include <ecs_manager.hpp>
#include <class.hpp>

namespace Entities_t
{
    struct Movable_t;
}

namespace Components
{

struct SnakeSegment_t
{
    ECS::Handle_t<Entities_t::Movable_t> target;
};

struct Position_t
{
    Vector2 position {  };
};

struct Transform_t
{
    Vector2 velocity {  };
};

}

namespace C = Components;

namespace Entities_t
{

struct Movable_t      : ECS::Class_t<C::Position_t>{  };
struct SnakeSegment_t : ECS::Class_t<C::SnakeSegment_t, Movable_t>{  };
struct SnakeTail_t    : ECS::Class_t<SnakeSegment_t>{  };
struct SnakeHead_t    : ECS::Class_t<Movable_t, C::Transform_t>{  };

}

namespace E = Entities_t;

struct ECSConfig {
    using Signatures_t = TMPL::TypeList_t<E::Movable_t, E::SnakeSegment_t, E::SnakeTail_t, E::SnakeHead_t>;
};

using ECSMan_t = ECS::ECSManager_t<ECSConfig>;

struct GameData
{
    ECS::Handle_t<E::SnakeHead_t> head;
    ECS::Handle_t<E::SnakeTail_t> tail;
    ECS::Handle_t<E::Movable_t> fruit;
};

int main(int argc, char *argv[])
{
    ECSMan_t ecs_man {  };
    auto head = ecs_man.CreateEntity<E::SnakeHead_t>(
            C::Position_t{ 40, 10 },
            C::Transform_t{ -1, 0 }
            );
    auto tail = ecs_man.CreateEntity<E::SnakeTail_t>(
            C::SnakeSegment_t{ ecs_man.GetBase<E::Movable_t>(head) },
            C::Position_t{ 41, 10 }
            );
    auto fruit = ecs_man.CreateEntity<E::Movable_t>();

    GameData g_data {
        .head  = head,
        .tail  = tail,
        .fruit = fruit
    };

    InitWindow(640, 480, "Snake");
    SetTargetFPS(5);

    while (not WindowShouldClose()) {

        if (IsKeyDown(KEY_SPACE)) {
            auto ss = ecs_man.TransformTo<E::SnakeSegment_t>(g_data.tail);
            g_data.tail = ecs_man.CreateEntity<E::SnakeTail_t>(
                    C::SnakeSegment_t{ ecs_man.GetBase<E::Movable_t>(ss) }
                    );
        }

        ecs_man.ASYNCForEach<E::SnakeSegment_t>([&](auto& ss, auto& pos, auto) {
                    auto& t_pos { ecs_man.GetComponent<C::Position_t>(ss.target) };
                    pos = t_pos;
                });

        ecs_man.ForEach<E::SnakeHead_t>(
                [&](auto& pos, auto& tr) {
                    pos.position.x += tr.velocity.x;
                    pos.position.y += tr.velocity.y;
                });

        auto& tr = ecs_man.GetComponent<C::Transform_t>(g_data.head);
        if (IsKeyDown(KEY_A)) {
            tr.velocity.x = -1;
        }
        if (IsKeyDown(KEY_D)) {
            tr.velocity.x = +1;
        }
        if (IsKeyDown(KEY_W)) {
            tr.velocity.y = -1;
        }
        if (IsKeyDown(KEY_S)) {
            tr.velocity.y = +1;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        ecs_man.ForEach<E::Movable_t>([&](auto& pos, auto) {
                    DrawRectangle(pos.position.x * 11, pos.position.y * 11, 10, 10, SKYBLUE);
                });

        //ecs_man.ForEach<E::Movable_t>(
        //        [&](auto& pos, auto) {
        //            DrawRectangle(pos.position.x * 11, pos.position.y * 11, 10, 10, RED);
        //        });

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
