#pragma once
#include <ZeusEngineCore/API.h>

#include "Event.h"
struct GLFWwindow;

namespace ZEN {
    struct CursorLockEvent;
    class EventDispatcher;
    class Window {
    public:
        Window(int width, int height, std::string title, ZEN::eRendererAPI api);

        void attachDispatcher();

        ~Window();

        void pollEvents();

        [[nodiscard]] float getDeltaTime() const;

        bool shouldClose();

        [[nodiscard]] GLFWwindow *getNativeWindow() const;

        float getWidth(){return m_Width;}

        float getHeight(){return m_Height;}

        void setCursorLock(bool isLocked);

        void updateWindowTitleWithFPS();

    private:
        void calculateDeltaTime();
        std::function<void(Event&)> m_SubmitEventFn{};

        GLFWwindow *m_Window = nullptr;
        int m_Width = 1280;
        int m_Height = 720;
        float m_LastTime;
        float m_DeltaTime;
        float m_SmoothFPS = 0.0f;
        float m_FPSAlpha = 0.1f;
        std::string m_Title;
    };
}
