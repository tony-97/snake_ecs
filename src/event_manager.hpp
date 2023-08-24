#pragma once

#include <functional>
#include <mutex>
#include <array>

#include <ecs_manager.hpp>
#include <class.hpp>

template<class Config_t>
class EventManager_t
{
public:
    template<class T>
    using Signal_t = std::function<void(T)>;

    template<class T>
    struct EventHandler_t : ECS::Class_t<Signal_t<T>> {  };

    using Mutexes = std::array<std::mutex, TMPL::Sequence::Size_v<typename Config_t::events>>;

    template<class Event_t, class Handler_t> constexpr auto
    suscribe(Handler_t&& handler) -> ECS::Handle_t<EventHandler_t<Event_t>>
    {
        return mEvents.template CreateEntity<EventHandler_t<Event_t>>(std::function<void(Event_t)>{ std::ref(std::forward<Handler_t>(handler)) });
    }

    template<class Event_t, class Handler_t>
    void unsuscribe(Handler_t& handler)
    {
        mEvents.template ForEachEntity<EventHandler_t<Event_t>>([&](auto ent) {
                    auto& signal = mEvents.template GetComponent<Signal_t<Event_t>>(ent);
                    if (std::addressof(handler) == signal.template target<void(Event_t)>()) {
                        mEvents.Destroy(ent);
                    }
                });
    }

    void unsuscribe(auto handler)
    {
        mEvents.Destroy(handler);
    }

    template<class Event_t>
    void publish(Event_t event)
    {
        std::lock_guard lock { mMutexes[TMPL::Sequence::IndexOf_v<Event_t, typename Config_t::events>] };
        mEvents.template ForEach<EventHandler_t<Event_t>>([ev=event,this](const auto& signal, auto) {
                    signal(ev);
                });
    }

private:

    template<class T>
    using ToEventHandler_t = std::type_identity<EventHandler_t<T>>; 

    struct ECSManConfig_t
    {
        using Signatures_t = TMPL::Sequence::Map_t<typename Config_t::events, ToEventHandler_t>;
    };

    ECS::ECSManager_t<ECSManConfig_t> mEvents {  };
    Mutexes mMutexes {  };
};

