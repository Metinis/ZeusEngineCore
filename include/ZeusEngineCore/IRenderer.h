#pragma once
#include <memory>
#include "ZeusEngineCore/IMesh.h"
#include "ZeusEngineCore/Material.h"

enum class RendererAPI;

class IRenderer {
public:
    virtual ~IRenderer() = default;

    virtual void Init() = 0;
    virtual void Cleanup() = 0;

    virtual void BeginFrame() = 0;
    virtual void Submit() = 0;
    virtual void EndFrame() = 0;

    virtual void DrawMesh(const IMesh& mesh, Material& material) = 0;

    static std::unique_ptr<IRenderer> Create(RendererAPI api);
};
