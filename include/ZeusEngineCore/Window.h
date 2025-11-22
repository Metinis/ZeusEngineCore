#pragma once
#include <ZeusEngineCore/API.h>
struct GLFWwindow;
namespace ZEN {
    struct CursorLockEvent;
    class EventDispatcher;
    class Window {
    public:
        Window(int width, int height, std::string title, ZEN::eRendererAPI api);

        void attachDispatcher(EventDispatcher& dispatcher);
        ~Window();

        void pollEvents();

        [[nodiscard]] float getDeltaTime() const;

        bool shouldClose();

        [[nodiscard]] GLFWwindow *getNativeWindow() const;

        float getWidth(){return m_Width;}

        float getHeight(){return m_Height;}

        void onCursorLockChange(CursorLockEvent &e);

        void updateWindowTitleWithFPS();

    private:
        void calculateDeltaTime();

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
