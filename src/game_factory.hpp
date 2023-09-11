#pragma once

#include "serialization.hpp"

#include <cmath>
#include <fstream>
#include <random>

struct GameFactory_t
{
  using json = nlohmann::json;

  explicit GameFactory_t(ECSMan_t& ecs_man, GameData_t& g_data)
    : mECSMan{ ecs_man }
    , mGData{ g_data }
    , mConfig(json::parse(std::ifstream("./game_config.json")))
    , mSpriteLayout(json::parse(std::ifstream("./texture_atlas.json")))
  {
    mGData               = mConfig["config"].get<GameData_t>();
    mGData.camera.offset = { .x = mGData.screen_width * 0.5f, .y = mGData.screen_height * 0.5f };
  }

  constexpr auto grow(auto e, unsigned inc) const -> void
  {
    mECSMan.Match<e::BasicCharacter_t>(e, [=](auto& ren, auto&, auto& col) {
      auto new_r{ col.size + inc };
      ren.scale = (ren.scale * new_r) / col.size;
      col.size  = new_r;
    });
  }

  template<class EntSig_t> constexpr auto ConfigureEntityFromJson(ECS::Handle_t<EntSig_t> e, const json& j) const
  {
    mECSMan.Match(
      e,
      [&](ECS::Handle_t<e::Renderable_t> eid) {
        auto& ren{ mECSMan.template GetComponent<c::Render_t>(eid) };
        auto sprite_name{ j["sprite_name"].get<std::string>() };
        auto& r{ mSpriteLayout["frames"][sprite_name]["frame"] };
        ren.crop = Rectangle{
          .x      = static_cast<float>(r["x"].get<int>()),
          .y      = static_cast<float>(r["y"].get<int>()),
          .width  = static_cast<float>(r["w"].get<int>()),
          .height = static_cast<float>(r["h"].get<int>()),
        };
      },
      [&](ECS::Handle_t<e::Collidable_t> eid) {
        auto& col{ mECSMan.template GetComponent<c::Collider_t>(eid) };
        col.size = j.get<c::Collider_t>().size;
      });
  }

  template<class EntSig_t> constexpr auto ConfigureEntityFromConfig(ECS::Handle_t<EntSig_t> e) const
  {
    constexpr auto type_id{ TMPL::Sequence::IndexOf_v<EntSig_t, ECSConfig_t::Signatures_t> };
    if (mConfig.contains(std::to_string(type_id))) {
      ConfigureEntityFromJson(e, mConfig[std::to_string(type_id)]);
    }
  }

  template<class EntSig_t, class... Args_t> constexpr auto EntityFromJSON(const json& j, Args_t&&... args) const -> auto
  {
    using Components_t = TMPL::Sequence::Difference_t<ECS::Traits::Components_t<EntSig_t>,
                                                      TMPL::TypeList_t<std::remove_cvref_t<Args_t>...>>;
    auto e             = TMPL::Sequence::Unpacker_t<Components_t>::Call(
      [&]<class... Ts, class... Cmps_t>(Cmps_t&&... cmps) {
        return mECSMan.template CreateEntity<EntSig_t>(j.get<Ts>()..., std::forward<Cmps_t>(cmps)...);
      },
      std::forward<Args_t>(args)...);
    ConfigureEntityFromJson(e, j);

    return e;
  }

  template<class EntSig_t, class Cmp_t> constexpr auto ComponentFromConfig() const -> auto
  {
    constexpr auto type_id{ TMPL::Sequence::IndexOf_v<EntSig_t, ECSConfig_t::Signatures_t> };
    if (mConfig.contains(std::to_string(type_id))) {
      return mConfig[std::to_string(type_id)].template get<Cmp_t>();
    } else {
      return Cmp_t{};
    }
  }

  template<class EntSig_t, class... Args_t> constexpr auto EntityFromConfig(Args_t&&... args) const -> auto
  {
    constexpr auto type_id{ TMPL::Sequence::IndexOf_v<EntSig_t, ECSConfig_t::Signatures_t> };
    if (mConfig.contains(std::to_string(type_id))) {
      return EntityFromJSON<EntSig_t>(mConfig[std::to_string(type_id)], std::forward<Args_t>(args)...);
    } else {
      return mECSMan.template CreateEntity<EntSig_t>();
    }
  }

  auto LoadScene(std::string_view file) const -> void
  {
    const json scene(json::parse(std::ifstream{ file.data() }));
    for (auto [k, j] : scene.items()) {
      int i{ std::stoi(k) };
      TMPL::Sequence::ForEach_t<ECSConfig_t::Signatures_t>::Do([&]<class T>() {
        if (i == TMPL::Sequence::IndexOf_v<T, ECSConfig_t::Signatures_t>) {
          EntityFromJSON<T>(j);
        }
      });
    }
  }

  auto LoadMainScene() const -> void
  {
    static std::mt19937                     gen(std::random_device{}());
    static std::uniform_real_distribution<> dis(0.0f, 1.0f);
    auto                                    head         = EntityFromConfig<e::SnakeHead_t>();
    auto                                    tail         = EntityFromConfig<e::SnakeTail_t>();
    mECSMan.GetComponent<c::SnakeSegment_t>(tail).target = mECSMan.GetBaseID<e::Collidable_t>(head);
    mECSMan.GetComponent<c::Render_t>(head).index = std::numeric_limits<unsigned int>::max();
    mECSMan.GetComponent<c::Render_t>(tail).index = std::numeric_limits<unsigned int>::max() - 1;
    grow(head, 2);

    for (unsigned i{}; i < 100000; ++i) {
      float r = 5000 * std::sqrt(dis(gen));
      float a = dis(gen) * 2 * M_PI;
      EntityFromConfig<e::Food_t>(
        c::Physics_t{
          r * std::sin(a),
          r * std::cos(a),
        },
        c::Render_t{ .color = { .r = static_cast<unsigned char>(GetRandomValue(0, 255)),
                                .g = static_cast<unsigned char>(GetRandomValue(0, 255)),
                                .b = static_cast<unsigned char>(GetRandomValue(0, 255)),
                                .a = 255 },
                     .scale = 0.625f,
                     .index = i });
    }
  }

private:
  ECSMan_t&   mECSMan;
  GameData_t& mGData;
  const json  mConfig{};
  const json  mSpriteLayout{};
};
