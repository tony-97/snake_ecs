#pragma once

#include <ecs_manager.hpp>

#include <entities.hpp>

struct ECSConfig
{
    using Signatures_t = TMPL::TypeList_t<e::Movable_t, e::Renderable_t, e::Collidable_t, e::Food_t, e::SnakeSegment_t, e::SnakeTail_t, e::SnakeHead_t>;
};

using ECSMan_t = ECS::ECSManager_t<ECSConfig>;
