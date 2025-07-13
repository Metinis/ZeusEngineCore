#pragma once
#include "ZeusEngineCore/ITexture.h"
#include "Backend/SampledImage.h"

namespace ZEN::VKAPI {

    class Texture : public ITexture {
    public:
        void Init(ZEN::TextureInfo &textureInfo) override;

        [[nodiscard]] vk::DescriptorImageInfo GetDescriptorInfo() const;

    private:
        std::optional<SampledImage> m_Image;
        vk::UniqueImageView m_View{};
        vk::UniqueSampler m_Sampler{};


    };
}