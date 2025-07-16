#pragma once
#include "IRendererAPI.h"
#include <memory>
#include "ZeusEngineCore/InfoVariants.h"

namespace ZEN {
    class ITexture {
    public:
        static std::shared_ptr<ITexture> Create(IRendererBackend* apiBackend,
                                                IRendererAPI* apiRenderer,
                                                const std::string& filepath);
        virtual void Bind() = 0;

        virtual ~ITexture() = default;
    private:
    };
}