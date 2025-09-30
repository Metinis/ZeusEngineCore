#pragma once
#include <entt.hpp>
#include <string>
#include <ZeusEngineCore/API.h>
struct GLFWwindow;
namespace ZEN {
    struct CursorLockEvent;
    class Window {
    public:
        Window(int width, int height, std::string title, ZEN::eRendererAPI api);

        void attachDispatcher(entt::dispatcher& dispatcher);
        ~Window();

        void pollEvents();

        [[nodiscard]] float getDeltaTime() const;

        bool shouldClose();

        [[nodiscard]] GLFWwindow *getNativeWindow() const;

        float getWidth(){return m_Width;}

        float getHeight(){return m_Height;}

        void onCursorLockChange(CursorLockEvent& e);

    private:
        void calculateDeltaTime();

        GLFWwindow *m_Window = nullptr;
        int m_Width = 1280;
        int m_Height = 720;
        float m_LastTime;
        float m_DeltaTime;
        std::string m_Title;
    };
}
