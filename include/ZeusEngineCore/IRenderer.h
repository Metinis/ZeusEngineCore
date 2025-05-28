#pragma once
#include <memory>
#include "ZeusEngineCore/IMesh.h"
#include "ZeusEngineCore/Material.h"

enum class RendererAPI;

struct RenderCommand {
    glm::mat4 transform;
    std::shared_ptr<Material> material;
    std::shared_ptr<IMesh> mesh;
};

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void Init() = 0;

    virtual void BeginFrame() = 0;
    virtual void Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) = 0;
    virtual void EndFrame() = 0;

    virtual void DrawMesh(const IMesh& mesh, Material& material) = 0;

    static std::unique_ptr<IRenderer> Create(RendererAPI api);

protected:
    std::vector<RenderCommand> m_RenderQueue;
};
