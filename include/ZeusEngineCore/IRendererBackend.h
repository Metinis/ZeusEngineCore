#pragma once
#include <memory>
#include <glm/glm.hpp>

namespace ZEN {
    enum class eRendererAPI;
    struct WindowHandle;

    class IRendererBackend {
    public:
        static std::unique_ptr<IRendererBackend> Create(eRendererAPI api, WindowHandle handle);
        virtual ~IRendererBackend() = default;
        [[nodiscard]] virtual eRendererAPI GetAPI() const = 0;
        [[nodiscard]] virtual glm::mat4 GetPerspectiveMatrix(float fov, float zNear, float zFar) const = 0;
    };

}