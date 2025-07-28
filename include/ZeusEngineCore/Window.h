#pragma once
#include <string>
struct GLFWwindow;
namespace ZEN {
    class Window {
    public:
        Window(int width, int height, std::string title, bool useVulkan);

        ~Window();

        void PollEvents();

        [[nodiscard]] float GetDeltaTime() const;

        bool ShouldClose();

        void SwapBuffers();

        [[nodiscard]] GLFWwindow *GetNativeWindow() const;

    private:
        void Init();

        void CalculateDeltaTime();

        GLFWwindow *m_Window = nullptr;
        int m_Width = 1280;
        int m_Height = 720;
        float m_LastTime;
        float m_DeltaTime;
        std::string m_Title;
        bool m_UseVulkan;
    };
}
