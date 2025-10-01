#pragma once
#include <entt.hpp>

namespace ZEN {
    class EventDispatcher {
    public:
        explicit EventDispatcher() = default;

        template<typename Event, typename C, void (C::*Handler)(Event&)>
        void attach(C* instance) {
            m_Handle.sink<Event>().template connect<Handler>(*instance);
        }

        template<typename Event, typename C, void (C::*Handler)(const Event&)>
        void attach(C* instance) {
            m_Handle.sink<Event>().template connect<Handler>(*instance);
        }

        template<typename T>
        void trigger(T&& t) {
            m_Handle.trigger<T>(std::forward<T>(t));
        }

    private:
        entt::dispatcher m_Handle{};
    };
}
