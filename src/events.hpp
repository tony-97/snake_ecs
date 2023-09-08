#pragma once

#include <type_aliases.hpp>

namespace e {

struct Collidable_t;

}

namespace ev {

struct Collision_t
{
  ECS::Handle_t<e::Collidable_t> e1;
  ECS::Handle_t<e::Collidable_t> e2;
};

}
