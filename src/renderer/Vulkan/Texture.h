#pragma once
#include "ZeusEngineCore/ITexture.h"
#include "Backend/SampledImage.h"
#include "Backend/CommandBlock.h"

namespace ZEN::VKAPI {
    class APIRenderer;
    [[nodiscard]] constexpr auto
    createSamplerCreateInfo(vk::SamplerAddressMode const wrap, vk::Filter const filter) {
        vk::SamplerCreateInfo createInfo{};
        createInfo.setAddressModeU(wrap);
        createInfo.setAddressModeV(wrap);
        createInfo.setAddressModeW(wrap);
        createInfo.setMinFilter(filter);
        createInfo.setMagFilter(filter);
        createInfo.setMaxLod(VK_LOD_CLAMP_NONE);
        createInfo.setBorderColor(vk::BorderColor::eFloatTransparentBlack);
        createInfo.setMipmapMode(vk::SamplerMipmapMode::eNearest);
        return createInfo;
    }

    constexpr auto samplerCreateInfo_v = createSamplerCreateInfo(
            vk::SamplerAddressMode::eClampToEdge, vk::Filter::eLinear);

    struct TextureInfo {
        vk::Device device{};
        VmaAllocator allocator{};
        std::uint32_t queueFamily{};
        std::optional<CommandBlock> commandBlock;
        vk::SamplerCreateInfo sampler{samplerCreateInfo_v};
        std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback;
        APIRenderer* apiRenderer;
        std::string filepath;
    };
    class Texture : public ITexture {

    public:
        explicit Texture(TextureInfo& texInfo);
        void Bind() override;
        [[nodiscard]] vk::DescriptorImageInfo GetDescriptorInfo() const;
        ~Texture() override;

    private:
        std::optional<SampledImage> m_Image;
        vk::UniqueImageView m_View{};
        vk::UniqueSampler m_Sampler{};
        APIRenderer* m_APIRenderer;


    };
}