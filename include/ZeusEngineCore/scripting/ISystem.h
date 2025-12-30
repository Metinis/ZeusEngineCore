#pragma once
#ifdef _WIN32
    #ifdef ENGINE_EXPORTS
        #define ZEN_API __declspec(dllexport)
    #else
        #define ZEN_API __declspec(dllimport)
    #endif
#else
    #define ZEN_API __attribute__((visibility("default")))
#endif
namespace ZEN {
    class Scene;

    class ZEN_API ISystem {
    public:
        virtual ~ISystem() = default;
        virtual void onUpdate(float dt) = 0;
        virtual void onLoad(Scene* scene) {
            m_Scene = scene;
        };
        virtual void onUnload() = 0;
    protected:
        Scene* m_Scene{};
    };
}