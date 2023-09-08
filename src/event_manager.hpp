#pragma once

#include "traits.hpp"
#include <array>
#include <functional>
#include <mutex>

#include <class.hpp>
#include <ecs_manager.hpp>

template<class Config_t> struct EventManager_t
{
private:
  template<class T> using Signal_t = std::function<void(T)>;

  template<class T> struct EventHandler_t : ECS::Class_t<Signal_t<T>>
  {};

  using Mutexes = std::array<std::mutex, TMPL::Sequence::Size_v<typename Config_t::events>>;

  template<class T> using ToEventHandler_t = std::type_identity<EventHandler_t<T>>;

  struct ECSManConfig_t
  {
    using Signatures_t = TMPL::Sequence::Map_t<typename Config_t::events, ToEventHandler_t>;
  };

public:
  constexpr explicit EventManager_t() = default;

  template<class Event_t, class Handler_t>
  constexpr auto suscribe(Handler_t& handler) -> ECS::Handle_t<EventHandler_t<Event_t>>
  {
    return mEvents.template CreateEntity<EventHandler_t<Event_t>>(std::function<void(Event_t)>{ std::ref(handler) });
  }

  template<class Event_t, class Handler_t> void unsuscribe(Handler_t& handler)
  {
    mEvents.template ForEachEntity<EventHandler_t<Event_t>>([&](auto ent) {
      auto& signal = mEvents.template GetComponent<Signal_t<Event_t>>(ent);
      if (std::addressof(handler) == signal.template target<void(Event_t)>()) {
        mEvents.Destroy(ent);
      }
    });
  }

  void unsuscribe(auto handler) { mEvents.Destroy(handler); }

  template<class Event_t> void publish(Event_t event) const
  {
    std::lock_guard lock{ mMutexes[TMPL::Sequence::IndexOf_v<Event_t, typename Config_t::events>] };
    mEvents.template ForEach<EventHandler_t<Event_t>>([ev = event](const auto& signal, auto) { signal(ev); });
  }

public:
  ECS::ECSManager_t<ECSManConfig_t> mEvents{};
  mutable Mutexes                   mMutexes{};
};
