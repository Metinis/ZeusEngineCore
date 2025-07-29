#pragma once
#include "ZeusEngineCore/ITexture.h"
#include <string>
#include <cstdint>

namespace ZEN::OGLAPI {
    class APIRenderer;
    struct TextureInfo{
        APIRenderer* apiRenderer;
        std::string filepath;
    };
    class Texture : public ITexture {
    public:
        //void Init(ZEN::TextureInfo &textureInfo) override;
        explicit Texture(const TextureInfo& texInfo);
        void Bind() override;
    private:
        std::uint32_t m_TextureHandle{};
    };

}