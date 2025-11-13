#pragma once

namespace ZEN {
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        virtual void onAttach() {}
        virtual void onDettach() {}

        virtual void onUpdate() {}

        virtual void onUIRender() {}

        virtual void onEvent() {}
    };
}