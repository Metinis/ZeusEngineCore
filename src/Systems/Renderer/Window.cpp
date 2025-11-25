#include "ZeusEngineCore/Window.h"
#include "GLFW/glfw3.h"
#include <ZeusEngineCore/API.h>
#include <ZeusEngineCore/Application.h>
#include <ZeusEngineCore/InputEvents.h>
#include <ZeusEngineCore/Event.h>

using namespace ZEN;


Window::Window(int width, int height, std::string title, ZEN::eRendererAPI api)
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

    m_SubmitEventFn = [](Event& event) {
        Application::get().callEvent(event);
    };
}

void Window::attachDispatcher() {
    glfwSetWindowUserPointer(m_Window, &m_SubmitEventFn);
    //m_Dispatcher->attach<SceneViewResizeEvent>(this, &CameraSystem::onResize);

    //dispatcher.attach<CursorLockEvent, Window, &Window::onCursorLockChange>(this);

    glfwSetFramebufferSizeCallback(m_Window, [](GLFWwindow* window, int width, int height) {
        auto* submitEventFn = static_cast<std::function<void(Event&)>*>(
            glfwGetWindowUserPointer(window)
        );
        WindowResizeEvent event(width, height);
        (*submitEventFn)(event);

    });

    glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scan,
        int action, int mods) {
            auto* submitEventFn = static_cast<std::function<void(Event&)>*>(
                glfwGetWindowUserPointer(window)
            );
        if(action == GLFW_PRESS) {
            KeyPressedEvent event(key, false);
            (*submitEventFn)(event);
        }
        if(action == GLFW_REPEAT) {
            KeyPressedEvent event(key, true);
            (*submitEventFn)(event);
        }
        if(action == GLFW_RELEASE) {
            KeyReleasedEvent event(key);
            (*submitEventFn)(event);
        }
    });
    glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods) {
        auto* submitEventFn = static_cast<std::function<void(Event&)>*>(
            glfwGetWindowUserPointer(window)
        );
        if(action == GLFW_PRESS) {
            MouseButtonPressedEvent event(button, false);
            (*submitEventFn)(event);
        }
        if(action == GLFW_REPEAT) {
            MouseButtonPressedEvent event(button, true);
            (*submitEventFn)(event);
        }
        if(action == GLFW_RELEASE) {
            MouseButtonReleasedEvent event(button);
            (*submitEventFn)(event);
        }
    });
    glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos) {
        auto* submitEventFn = static_cast<std::function<void(Event&)>*>(
            glfwGetWindowUserPointer(window)
        );
        MouseMovedEvent event(xPos, yPos);
        (*submitEventFn)(event);
    });
}

Window::~Window() {
    if(m_Window) {
        glfwDestroyWindow(m_Window);
        glfwTerminate();
    }
}

void Window::updateWindowTitleWithFPS() {
    float currentFPS = 1.0f / m_DeltaTime;

    if (m_SmoothFPS == 0.0f)
        m_SmoothFPS = currentFPS;
    else
        m_SmoothFPS = m_SmoothFPS + m_FPSAlpha * (currentFPS - m_SmoothFPS);

    char titleBuffer[128];
    snprintf(titleBuffer, sizeof(titleBuffer), "%s - FPS: %.1f", m_Title.c_str(), m_SmoothFPS);

    glfwSetWindowTitle(m_Window, titleBuffer);
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
    updateWindowTitleWithFPS();
}

void Window::setCursorLock(bool isLocked, int xPos, int yPos) {
    if (isLocked) {
        glfwSetCursorPos(m_Window, xPos, yPos);
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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




