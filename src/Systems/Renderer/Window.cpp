#include "ZeusEngineCore/Window.h"
#include <stdexcept>
#include <utility>
#include "GLFW/glfw3.h"
#include <ZeusEngineCore/API.h>
#include <ZeusEngineCore/InputEvents.h>

using namespace ZEN;


Window::Window(int width, int height, std::string title, ZEN::eRendererAPI api,
    entt::dispatcher& dispatcher)
    : m_Width(width), m_Height(height), m_Title(std::move(title)) {
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");

    if (api != ZEN::OpenGL) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    }
    else {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
#ifdef __APPLE__
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on macOS
#else
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
#endif
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);

    if (!m_Window)
        throw std::runtime_error("Failed to create window");

    m_LastTime = static_cast<float>(glfwGetTime());

    glfwSetWindowUserPointer(m_Window, &dispatcher);

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        auto* disp = static_cast<entt::dispatcher*>(
            glfwGetWindowUserPointer(window)
        );
        disp->trigger<ZEN::WindowResizeEvent>({width, height});
    });
}
Window::~Window() {
    if(m_Window) {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
}

void Window::pollEvents() {
    glfwPollEvents();
    calculateDeltaTime();
    int width, height;
    glfwGetFramebufferSize(m_Window, &width, &height);

    if (width != m_Width || height != m_Height) {
        m_Width = width;
        m_Height = height;
    }
}
void Window::calculateDeltaTime() {
    float currentTime = static_cast<float>(glfwGetTime());
    m_DeltaTime = currentTime - m_LastTime;
    m_LastTime = currentTime;
}
float Window::getDeltaTime() const {
    return m_DeltaTime;
}
GLFWwindow* Window::getNativeWindow() const {
    return m_Window;
}
bool Window::shouldClose() {
    return glfwWindowShouldClose(m_Window);
}




