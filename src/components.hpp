#pragma once

#include <raylib.h>

#include <type_aliases.hpp>

namespace e
{

struct Collidable_t;

} // namespace entities

namespace c
{

struct SnakeSegment_t
{
    ECS::Handle_t<e::Collidable_t> target { 0 };
    bool dummy { };
};

struct Health_t
{
    bool alive { true };
};

struct Physics_t
{
    Vector2 position {  };
    Vector2 velocity {  };
    Vector2 direction {  };
};

struct Render_t
{
    Color color { RAYWHITE };
    float scale { 1.0f };
    Rectangle crop { 0, 0, 64, 64 };
    unsigned index {  };
};

struct Collider_t
{
    unsigned size {  };
    unsigned mask {  };
    unsigned id   {  };
};

} // namespace components
