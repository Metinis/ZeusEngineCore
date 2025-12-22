#pragma once

namespace ZEN {
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
