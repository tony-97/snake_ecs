#pragma once

#include <raylib.h>

#include <ecs_manager.hpp>

#include "entities.hpp"
#include "event_manager.hpp"
#include "events.hpp"
#include "quad_tree.hpp"
#include "resource_manager.hpp"

using QdTree_t = QuadTree_t<ECS::Handle_t<e::Collidable_t>, 4>;

struct GameData_t
{
  ECS::Handle_t<e::SnakeHead_t> head{ 0 };
  ECS::Handle_t<e::SnakeTail_t> tail{ 0 };
  unsigned                      screen_width{ 1080 };
  unsigned                      screen_height{ 720 };

  unsigned segment_size{ 32 };
  unsigned speed{ 150 };
  unsigned score{};

  Camera2D         camera{ .offset = { screen_width * 0.5f, screen_width * 0.5f }, .rotation = 0, .zoom = 1 };
  mutable QdTree_t qd_tree{ Rectangle{ -5000.0f, -5000.0f, 10000.0f, 10000.0f } };
};

struct ECSConfig_t
{
  using Signatures_t = TMPL::TypeList_t<e::Movable_t,
                                        e::Alive_t,
                                        e::Renderable_t,
                                        e::Collidable_t,
                                        e::BasicCharacter_t,
                                        e::LivingCharacter_t,
                                        e::Food_t,
                                        e::SnakeSegment_t,
                                        e::SnakeTail_t,
                                        e::SnakeHead_t,
                                        e::move_tag_t>;
};

using ECSMan_t = ECS::ECSManager_t<ECSConfig_t>;

struct EVConfig_t
{
  using events = TMPL::TypeList_t<ev::Collision_t>;
};

using EVMan_t = EventManager_t<EVConfig_t>;
