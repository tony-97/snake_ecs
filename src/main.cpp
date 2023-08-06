#include <algorithm>
#include <raylib.h>
#include <raymath.h>

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

struct Render_t
{

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

struct ECSConfig
{
    using Signatures_t = TMPL::TypeList_t<E::Movable_t, E::SnakeSegment_t, E::SnakeTail_t, E::SnakeHead_t>;
};

using ECSMan_t = ECS::ECSManager_t<ECSConfig>;

struct GameData
{
    ECS::Handle_t<E::SnakeHead_t> head;
    ECS::Handle_t<E::SnakeTail_t> tail;
    ECS::Handle_t<E::Movable_t> fruit;
    unsigned screen_width  { 1080 };
    unsigned screen_height { 720 };
    static constexpr int snake_size { 32 };
    static constexpr int velocity   { 300 };
};

int main()
{
    ECSMan_t ecs_man {  };
    auto head = ecs_man.CreateEntity<E::SnakeHead_t>();
    auto tail = ecs_man.CreateEntity<E::SnakeTail_t>(
            C::SnakeSegment_t{
                .target = ecs_man.GetBase<E::Movable_t>(head),
            }
            );
    auto fruit = ecs_man.CreateEntity<E::Movable_t>();

    GameData g_data {
        .head  = head,
        .tail  = tail,
        .fruit = fruit
    };
    unsigned tint { 64 };

    Camera2D camera
    {
        .offset = { .x = g_data.screen_width * 0.5f, .y = g_data.screen_height * 0.5f },
        .target = ecs_man.GetComponent<C::Position_t>(g_data.head).position,
        .rotation = 0.0f,
        .zoom = 1.0f
    };

    InitWindow(g_data.screen_width, g_data.screen_height, "Snake");

    auto segment { LoadTexture("./segment.png") };

    while (not WindowShouldClose()) {
        if (IsKeyPressed(KEY_SPACE)) {
            auto pos = ecs_man.GetComponent<C::Position_t>(g_data.tail);
            auto ss = ecs_man.TransformTo<E::SnakeSegment_t>(g_data.tail);
            g_data.tail = ecs_man.CreateEntity<E::SnakeTail_t>(
                    C::SnakeSegment_t{ ecs_man.GetBase<E::Movable_t>(ss) },
                    C::Position_t{ pos.position }
                    );
        }

        ecs_man.Match<E::SnakeHead_t>(g_data.head,
                [&](auto& pos, auto& tr) {
                    auto mouse { GetMousePosition() };
                    auto direction { Vector2Normalize(Vector2Subtract(mouse, pos.position)) };
                    auto velocity { Vector2Scale(direction, GetFrameTime() * g_data.velocity) };
                    pos.position = Vector2Add(pos.position, velocity);
                    camera.target = pos.position;
                });

        //auto head { ecs_man.GetBase<E::Movable_t>(g_data.head) };
        //auto next { ecs_man.GetBase<E::Movable_t>(g_data.tail) };
        //while (next.GetIndex() != head.GetIndex()) {
        //    ecs_man.Match<E::SnakeSegment_t>(next, [&](auto& ss, auto& pos) {
        //                auto& t_pos { ecs_man.GetComponent<C::Position_t>(ss.target) };
        //                if (Vector2Distance(pos.position, t_pos.position) > g_data.snake_size) {
        //                    Vector2 dir { Vector2Subtract(pos.position, t_pos.position) };
        //                    dir = Vector2Normalize(dir);
        //                    pos.position = Vector2Add(t_pos.position, Vector2Scale(dir, g_data.snake_size));
        //                }
        //                next = ss.target;
        //            });
        //}

        ecs_man.ParallelForEach<E::SnakeSegment_t>([&](auto& ss, auto& pos, auto) {
                    auto& t_pos { ecs_man.GetComponent<C::Position_t>(ss.target) };
                    if (Vector2Distance(pos.position, t_pos.position) > g_data.snake_size) {
                        Vector2 dir { Vector2Subtract(pos.position, t_pos.position) };
                        dir = Vector2Normalize(dir);
                        pos.position = Vector2Add(t_pos.position, Vector2Scale(dir, g_data.snake_size));
                    }
                });

        BeginDrawing();
        ClearBackground(BLACK);

        BeginMode2D(camera);
        ecs_man.Match<E::Movable_t>(g_data.fruit,
                [&](auto& pos, auto) {
                    DrawCircleV(pos.position, g_data.snake_size * 0.25f, RED);
                });

        auto size { ecs_man.Size<E::SnakeSegment_t>() };
        ecs_man.ForEach<E::SnakeSegment_t>([&](auto& ss, auto& pos, auto) {
                    Rectangle src_rec {  };
                    src_rec.width = segment.width; src_rec.height = segment.height;
                    Rectangle dest_rec = src_rec;
                    dest_rec.x = pos.position.x; dest_rec.y = pos.position.y;
                    DrawTexturePro(segment, src_rec, dest_rec, { segment.width * 0.5f, segment.height * 0.5f }, 0.0f, RAYWHITE);
                });

        ecs_man.Match<E::Movable_t>(g_data.head,
                [&](auto& pos, auto) {
                    Rectangle src_rec {  };
                    src_rec.width = segment.width; src_rec.height = segment.height;
                    Rectangle dest_rec = src_rec;
                    dest_rec.x = pos.position.x; dest_rec.y = pos.position.y;
                    DrawTexturePro(segment, src_rec, dest_rec, { segment.width * 0.5f, segment.height * 0.5f }, 0.0f, RED);
                });

        EndMode2D();
        DrawFPS(10, 10);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
