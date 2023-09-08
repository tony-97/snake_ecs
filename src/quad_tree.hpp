#pragma once

#include <ecs_map.hpp>

#include <algorithm>
#include <vector>

template<class T>
concept RectangleV1 = requires(T t) {
  {
    t.x
  };
  {
    t.y
  };
  {
    t.width
  };
  {
    t.height
  };
};

template<class T>
concept RectangleV2 = requires(T t) {
  {
    t.x
  };
  {
    t.y
  };
  {
    t.w
  };
  {
    t.h
  };
};

template<class T> struct Rectangle_t
{
  T x{};
  T y{};
  T width{};
  T height{};

  constexpr Rectangle_t() = default;

  constexpr Rectangle_t(T px, T py, T w, T h)
    : x{ px }
    , y{ py }
    , width{ w }
    , height{ h }
  {
  }

  constexpr Rectangle_t(RectangleV1 auto r)
    : Rectangle_t(r.x, r.y, r.width, r.height)
  {
  }

  constexpr Rectangle_t(RectangleV2 auto r)
    : Rectangle_t(r.x, r.y, r.w, r.h)
  {
  }

  constexpr auto overlaps(Rectangle_t r2) const -> bool
  {
    auto& r1{ *this };
    return r1.x + r1.width >= r2.x &&  // r1 right edge past r2 left
           r1.x <= r2.x + r2.width &&  // r1 left edge past r2 right
           r1.y + r1.height >= r2.y && // r1 top edge past r2 bottom
           r1.y <= r2.y + r2.height;
  }

  constexpr auto contains(Rectangle_t r2) const -> bool
  {
    auto& r1{ *this };
    return r1.x <= r2.x && r2.x + r2.width <= r1.x + r1.width && r1.y <= r2.y && r2.y + r2.height <= r1.y + r1.height;
  }

  constexpr friend auto operator==(Rectangle_t r1, Rectangle_t r2) -> bool
  {
    return r1.x == r2.x && r1.y == r2.y && r1.width == r2.width && r1.height == r2.height;
  }

  template<RectangleV1 R> constexpr operator R() const
  {
    return R(static_cast<decltype(std::declval<R>().x)>(x),
             static_cast<decltype(std::declval<R>().y)>(y),
             static_cast<decltype(std::declval<R>().width)>(width),
             static_cast<decltype(std::declval<R>().height)>(height));
  }

  template<RectangleV2 R> constexpr operator R() const
  {
    return R(static_cast<decltype(std::declval<R>().x)>(x),
             static_cast<decltype(std::declval<R>().y)>(y),
             static_cast<decltype(std::declval<R>().w)>(width),
             static_cast<decltype(std::declval<R>().h)>(height));
  }
};

template<class T, unsigned MAX_DEPTH = 8, unsigned COLUMNS = 2, unsigned ROWS = 2> struct QuadTree_t
{
  using Recf_t    = Rectangle_t<float>;
  using KeyItem_t = typename ECS::ECSMap_t<struct QuadItem_t>::Key_t;
  using KeyKey_t  = typename ECS::ECSMap_t<struct QuadKey_t>::Key_t;

  struct QuadKey_t
  {
    Recf_t    area{};
    KeyItem_t key{};
  };

  struct QuadItem_t
  {
    T                         value{};
    ECS::ECSMap_t<QuadKey_t>* keys{};
    KeyKey_t                  key{};
  };

  struct Inserted_t
  {
    ECS::ECSMap_t<QuadKey_t>& keys{};
    KeyKey_t                  key{};
  };

  struct QuadNode_t
  {
    unsigned                 mDepth{ 0 };
    Recf_t                   mArea{};
    std::vector<QuadNode_t>  mLeafs{};
    ECS::ECSMap_t<QuadKey_t> mKeys{};

    constexpr QuadNode_t(Recf_t area, unsigned depth = 0)
      : mDepth{ depth }
      , mArea{ area }
    {
      mLeafs.reserve(COLUMNS * ROWS);
      // mKeys.reserve(64);
    }

    constexpr auto resize(Recf_t area) -> void
    {
      clear();
      mArea = area;
    }

    constexpr auto clear() -> void
    {
      mKeys.clear();
      mLeafs.clear();
    }

    constexpr auto insert(KeyItem_t k, Recf_t area) -> Inserted_t
    {
      const auto width{ mArea.width / COLUMNS };
      const auto height{ mArea.height / ROWS };
      for (float x{ mArea.x }; x < mArea.x + mArea.width; x += width) {
        for (float y{ mArea.y }; y < mArea.y + mArea.height; y += height) {
          Recf_t leaf_area{ x, y, width, height };
          if (mDepth <= MAX_DEPTH && leaf_area.contains(area)) {
            auto it{ std::find_if(mLeafs.begin(), mLeafs.end(), [&](auto&& leaf) { return leaf.mArea == leaf_area; }) };
            if (it != mLeafs.end()) {
              return it->insert(k, area);
            } else {
              auto& leaf{ mLeafs.emplace_back(leaf_area, mDepth + 1) };
              return leaf.insert(k, area);
            }
          }
        }
      }
      auto keykey{ mKeys.emplace_back(area, k).key() };
      return { mKeys, { keykey } };
    }

    constexpr auto nodes(std::vector<const QuadNode_t*>& nds) const -> void
    {
      for (auto&& leaf : mLeafs) {
        nds.push_back(&leaf);
      }
      for (auto&& leaf : mLeafs) {
        leaf.nodes(nds);
      }
    }

    constexpr auto quads(Recf_t area, std::vector<const QuadNode_t*>& nodes) const -> void
    {
      if (mArea.overlaps(area)) {
        nodes.push_back(this);
      }
      for (auto&& leaf : mLeafs) {
        if (area.contains(leaf.mArea)) {
          nodes.push_back(&leaf);
          leaf.nodes(nodes);
        } else if (leaf.mArea.overlaps(area)) {
          leaf.quads(area, nodes);
        }
      }
    }

    constexpr auto quads(Recf_t area) const -> std::vector<const QuadNode_t*>
    {
      std::vector<const QuadNode_t*> nodes{};
      quads(area, nodes);
      return nodes;
    }

    constexpr auto keys(std::vector<KeyItem_t>& keys) const -> void
    {
      for (auto&& k : mKeys) {
        keys.push_back(k.value().key);
      }
      for (auto&& leaf : mLeafs) {
        leaf.keys(keys);
      }
    }

    constexpr auto search(Recf_t area, std::vector<KeyItem_t>& keys) const -> void
    {
      for (auto&& k : mKeys) {
        if (area.overlaps(k.value().area)) {
          keys.push_back(k.value().key);
        }
      }
      for (auto&& leaf : mLeafs) {
        if (area.contains(leaf.mArea)) {
          leaf.keys(keys);
        } else if (area.overlaps(leaf.mArea)) {
          leaf.search(area, keys);
        }
      }
    }

    constexpr auto search(Recf_t area) const -> std::vector<KeyItem_t>
    {
      std::vector<KeyItem_t> items{};
      search(area, items);
      return items;
    }
  };

  constexpr explicit QuadTree_t(Recf_t area)
    : mRoot{ area }
  {
  }

  constexpr auto resize(Recf_t area) -> void
  {
    mRoot.resize(area);
    mItems.clear();
  }

  constexpr auto clear() -> void
  {
    mRoot.clear();
    mItems.clear();
  }

  constexpr auto insert(const T& obj, Recf_t area) -> KeyItem_t
  {
    auto& slot{ mItems.emplace_back(std::move(obj)) };
    auto [keys, keykey]{ mRoot.insert({ slot.key() }, area) };
    slot.value().keys = &keys;
    slot.value().key  = keykey;
    return { slot.key() };
  }

  constexpr auto erase(KeyItem_t key) -> void
  {
    auto& item{ mItems[key] };
    item.keys->erase({ item.key });
    mItems.erase({ key });
  }

  constexpr auto update(Recf_t area, KeyItem_t key) -> void
  {
    auto& item{ mItems[key] };
    item.keys->erase({ item.key });
    auto [keys, keykey]{ mRoot.insert({ key }, area) };
    item.keys = &keys;
    item.key  = keykey;
  }

  constexpr auto quads(Recf_t area) const -> std::vector<const QuadNode_t*> { return mRoot.quads(area); }

  constexpr auto search(Recf_t area) const -> std::vector<KeyItem_t> { return mRoot.search(area); }

  constexpr auto operator[](KeyItem_t key) -> T& { return mItems[key].value; }

  constexpr auto operator[](KeyItem_t key) const -> const T& { return mItems[key].value; }

  QuadNode_t                mRoot{};
  ECS::ECSMap_t<QuadItem_t> mItems{};
};
