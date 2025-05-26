#pragma once
#include <memory>
#include <glm/vec4.hpp>

enum class RendererAPI;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void Init() = 0;
    virtual void Cleanup() = 0;

    virtual void BeginFrame() = 0;
    virtual void Submit() = 0;
    virtual void EndFrame() = 0;

    virtual void DrawMesh(glm::vec4 color) = 0;

    static std::unique_ptr<IRenderer> Create(RendererAPI api);
};
