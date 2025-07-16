#pragma once
#include "ZeusEngineCore/IRendererBackend.h"

namespace ZEN{
    enum class eDescriptorBufferType;
}
namespace ZEN::OGLAPI {
    struct MeshInfo;
    struct TextureInfo;
    struct BackendInfo;
    struct ShaderInfo;
    struct APIRenderer;
    struct BufferCreateInfo{
        APIRenderer *apiRenderer;
    };
    class APIBackend : public IRendererBackend{
    public:
        explicit APIBackend() = default;

        [[nodiscard]] eRendererAPI GetAPI() const override;

        [[nodiscard]] BackendInfo GetInfo() const;

        [[nodiscard]] MeshInfo GetMeshInfo() const;

        [[nodiscard]] ShaderInfo GetShaderInfo() const;

        [[nodiscard]] TextureInfo GetTextureInfo() const;

        [[nodiscard]] BufferCreateInfo GetBufferCreateInfo(eDescriptorBufferType type) const;
    };
}

