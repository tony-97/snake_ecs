#pragma once

#include <class.hpp>

#include <components.hpp>

namespace e
{

struct Alive_t           : ECS::Class_t<c::Health_t>{  };
struct Movable_t         : ECS::Class_t<c::Physics_t>{  };
struct Renderable_t      : ECS::Class_t<c::Render_t, Movable_t>{  };
struct Collidable_t      : ECS::Class_t<Movable_t, c::Collider_t>{  };
struct BasicCharacter_t  : ECS::Class_t<Renderable_t, Collidable_t>{  };
struct LivingCharacter_t : ECS::Class_t<BasicCharacter_t, Alive_t>{  };
struct Food_t            : ECS::Class_t<LivingCharacter_t>{  };
struct SnakeSegment_t    : ECS::Class_t<BasicCharacter_t, c::SnakeSegment_t>{  };
struct SnakeTail_t       : ECS::Class_t<BasicCharacter_t, SnakeSegment_t>{  };
struct SnakeHead_t       : ECS::Class_t<LivingCharacter_t>{  };

} // namespace entities
