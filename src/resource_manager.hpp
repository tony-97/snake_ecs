#pragma once

#include <raylib.h>

#include <helpers.hpp>

struct ResourceManager_t : ECS::Uncopyable_t
{
  explicit ResourceManager_t() = default;

  ~ResourceManager_t() { UnloadTexture(mAtlas); }

  const Texture2D mAtlas{ LoadTexture("resources/sprites/texture_atlas.png") };
};
