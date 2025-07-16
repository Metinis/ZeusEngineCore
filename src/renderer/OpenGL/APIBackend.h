#pragma once
#include "ZeusEngineCore/IRendererBackend.h"
#include "Shader.h"

namespace ZEN::OGLAPI {
    struct MeshInfo;
    struct TextureInfo;
    class APIBackend : public IRendererBackend{
    public:
        explicit APIBackend(){};

        eRendererAPI GetAPI() const override;

        BackendInfo GetInfo() const;

        MeshInfo GetMeshInfo() const;

        ShaderInfo GetShaderInfo() const;

        TextureInfo GetTextureInfo() const;
    };
}

