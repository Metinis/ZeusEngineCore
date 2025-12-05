#pragma once
//#include <entt.hpp>

namespace ZEN {
    /*class EventDispatcher {
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
    };*/
    /*
    enum class EventType
    {
        None = 0,
        WindowClose, WindowResize,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
    };


#define EVENT_CLASS_TYPE(type) static EventType GetStaticType() { return EventType::type; }\
                                virtual EventType GetEventType() const override { return GetStaticType(); }\
                                virtual const char* GetName() const override { return #type; }

    class Event
    {
    public:
        bool Handled = false;

        virtual ~Event() {}
        virtual EventType GetEventType() const = 0;
        virtual const char* GetName() const = 0;
        virtual std::string ToString() const { return GetName(); }
    };
     **/
#define EVENT_CLASS_TYPE(type)  static EventType getStaticType() { return EventType::type; }\
                                virtual EventType getEventType() const override { return getStaticType(); }\
                                virtual const char* getName() const override { return #type; }
    enum class EventType {
        None = 0,
        WindowResize, WindowClose, ViewportResize,
        KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
        RemoveResource,
        RunPlayMode
    };

    class Event {
    public:
        bool handled{false};
        virtual ~Event() {}
        virtual EventType getEventType() const = 0;
        virtual const char* getName() const = 0;
        virtual std::string toString() const { return getName(); }
    };
    class EventDispatcher {
        template<typename T>
        using eventFn = std::function<bool(T&)>;

    public:
        EventDispatcher(Event& event) : m_Event(event) {}

        template<typename T>
        bool dispatch(eventFn<T> func) {
			//When we create the dispatcher, the function attached here will be called if its the same event type as declared with
            if (m_Event.getEventType() == T::getStaticType() && !m_Event.handled)
            {
                m_Event.handled = func(*(T*)&m_Event);
                return true;
            }
            return false;
        }
    private:
        Event& m_Event;
    };
}
