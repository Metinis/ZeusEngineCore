#pragma once
#include "RendererAPI.h"
#include <memory>
#include "ZeusEngineCore/InfoVariants.h"

namespace ZEN {

    struct TextureInfo {
        std::string path;
        TextureInfoVariant textInfoVariant;
    };

    class ITexture {
    public:
        virtual void Init(TextureInfo &textureInfo) = 0;

        static std::shared_ptr<ITexture> Create(RendererAPI api);

    private:
    };
}