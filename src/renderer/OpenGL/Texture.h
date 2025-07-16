#pragma once
#include "ZeusEngineCore/ITexture.h"

namespace ZEN::OGLAPI {
    class APIRenderer;
    struct TextureInfo{
        APIRenderer* apiRenderer;
        std::string filepath;
    };
    class Texture : public ITexture {
    public:
        //void Init(ZEN::TextureInfo &textureInfo) override;
        Texture(const TextureInfo& texInfo);
    };
}