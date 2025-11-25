#pragma once
#include "Event.h"

namespace ZEN {
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual void onAttach() {}
        virtual void onDettach() {}

        virtual void onUpdate(float dt) {}

        virtual void onUIRender() {}
        virtual void onRender() {}

        virtual void onEvent(Event& event) {}

    };
}
