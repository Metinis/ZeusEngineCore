#pragma once
#include "ZeusEngineCore/ITexture.h"

namespace ZEN::OGLAPI {
    class Texture : public ITexture {
    public:
        void Init(ZEN::TextureInfo &textureInfo) override;
    };
}