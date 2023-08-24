#pragma once

#include <nlohmann/json.hpp>

#include "serialization.hpp"

#include <fstream>
#include <random>
#include <cmath>

struct GameFactory_t
{
    using json = nlohmann::json;

    explicit GameFactory_t(ECSMan_t& ecs_man)
        : mECSMan { ecs_man },
          mConfig(json::parse(std::ifstream("./game_config.json"))),
          mSpriteLayout(json::parse(std::ifstream("./atlas_layout.json"))) {  };

    constexpr auto grow(auto e, unsigned inc)
    {
        mECSMan.Match<e::BasicCharacter_t>(e, [=](auto& ren, auto&, auto& col) {
                    auto new_r { col.size + inc };
                    ren.scale = (ren.scale * new_r) / col.size;
                    col.size = new_r;
                });
    }

    template<class EntSig_t>
    auto EntityFromJSON(const json& j) -> auto
    {
        auto e = TMPL::Sequence::Unpacker_t<ECS::Traits::Components_t<EntSig_t>>::Call([&]<class... Ts>() {
                                 return mECSMan.template CreateEntity<EntSig_t>(j.get<Ts>()...);
                 });
            mECSMan.Match(e,
            [&](ECS::Handle_t<e::Renderable_t> eid) {
                auto& ren { mECSMan.template GetComponent<c::Render_t>(eid)  };
                auto sprite_name { j["sprite_name"].get<std::string>() };
                auto r { mSpriteLayout["frames"][sprite_name]["frame"] };
                ren.crop = Rectangle {
                        .x      = static_cast<float>(r["x"].get<int>()),
                        .y      = static_cast<float>(r["y"].get<int>()),
                        .width  = static_cast<float>(r["w"].get<int>()),
                        .height = static_cast<float>(r["h"].get<int>()),
                    };
            });//,
            //[&](ECS::Handle_t<Animable_t> eid) {
            //    auto& anim { mECSMan.template GetComponent<AnimationComponent_t>(eid)  };
            //    auto& ren { mECSMan.template GetComponent<RenderComponent_t>(eid)  };
            //    auto frame { j["sprite_name"].get<std::string>() };
            //    unsigned i {  };
            //    for (std::string next_frame { frame + "/" + std::to_string(i) };
            //         mSpriteLayout["frames"].contains(next_frame);
            //         next_frame = frame + "/" + std::to_string(i)) {
            //        auto r { mSpriteLayout["frames"][next_frame]["frame"] };
            //        anim.frames.emplace_back(Rectangle {
            //                    .x      = static_cast<float>(r["x"].get<int>()),
            //                    .y      = static_cast<float>(r["y"].get<int>()),
            //                    .width  = static_cast<float>(r["w"].get<int>()),
            //                    .height = static_cast<float>(r["h"].get<int>()),
            //                });
            //    }
            //});

            return e;
    }

    template<class EntSig_t>
    auto EntityFromConfig() -> auto
    {
        constexpr auto type_id { TMPL::Sequence::IndexOf_v<EntSig_t, ECSConfig_t::Signatures_t> };
        if (mConfig.contains(std::to_string(type_id))) {
            return EntityFromJSON<EntSig_t>(mConfig[std::to_string(type_id)]);
        } else {
            return mECSMan.template CreateEntity<EntSig_t>();
        }
    }

    auto LoadScene(std::string_view file) -> void
    {
        json scene(json::parse(std::ifstream{ file.data() }));
        for (auto [k, j] : scene.items()) {
            int i { std::stoi(k) };
            TMPL::Sequence::ForEach_t<ECSConfig_t::Signatures_t>::Do([&]<class T>() {
                if (i == TMPL::Sequence::IndexOf_v<T, ECSConfig_t::Signatures_t>) {
                    EntityFromJSON<T>(j);
                }
            });
        }
    }

    GameData_t LoadMainScene(unsigned screen_width, unsigned screen_height)
    {
        auto g_data = mConfig["config"].get<GameData_t>();
        static std::mt19937 gen(std::random_device{  }());
        static std::uniform_real_distribution<> dis(0.0f, 1.0f);
        unsigned size = 8;
        float scale = size * 1.0f / g_data.segment_size;
        
        auto head = EntityFromConfig<e::SnakeHead_t>();
        grow(head, 2);
        auto tail = EntityFromConfig<e::SnakeHead_t>();
        mECSMan.GetComponent<c::SnakeSegment_t>(tail).target = mECSMan.GetBase<e::Collidable_t>(head);

        for (unsigned i {  }; i < 20000; ++i) {
            float r = 5000 * std::sqrt(dis(gen));
            float a = dis(gen) * 2 * M_PI;
            mECSMan.CreateEntity<e::Food_t>(
                c::Physics_t {
                r * std::sin(a),
                r * std::cos(a),
                },
                c::Render_t {
                    .color = {
                        .r = static_cast<unsigned char>(GetRandomValue(0, 255)),
                        .g = static_cast<unsigned char>(GetRandomValue(0, 255)),
                        .b = static_cast<unsigned char>(GetRandomValue(0, 255)),
                        .a = 255
                    },
                    .scale = 0.125f
                },
                c::Collider_t {
                    .size = static_cast<unsigned>(g_data.segment_size / 8),
                }
                );
        }

        return g_data;
    }

private:
    ECSMan_t& mECSMan;
    const json mConfig {  };
    const json mSpriteLayout {  };
};
