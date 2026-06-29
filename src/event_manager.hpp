#pragma once

#include "struct_of_arrays.hpp"
#include "tmpl/sequence.hpp"
#include <tuple>
#include <utility>
#include <vector>

template<class Config_t> struct EventManager_t
{
private:
  template<class... Evs_t> using EventBus = ECS::SoA_t<std::vector, Evs_t...>;

  using EventBus_t = TMPL::Sequence::As_t<EventBus, typename Config_t::events>;

public:
  constexpr explicit EventManager_t() = default;

  template<class Event_t> constexpr auto publish(Event_t&& event) -> void
  {
    mEvents.template emplace_back<Event_t>(std::forward<Event_t>(event));
  }

  template<class Event_t> constexpr auto get_events() const
  {
    return static_cast<const std::vector<Event_t>&>(mEvents);
  }

  constexpr auto clear() -> void
  {
    TMPL::Sequence::ForEach_t<typename Config_t::events>::Do(
      [&]<class Event_t>() { mEvents.template clear<Event_t>(); });
  }

private:
  EventBus_t mEvents{};
};
