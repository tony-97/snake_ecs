#pragma once

#include <raylib.h>

struct ResourceManager_t
{
  explicit ResourceManager_t() = default;

  ~ResourceManager_t() { UnloadTexture(mAtlas); }

  const Texture2D mAtlas{ LoadTexture("./texture_atlas.png") };
};
