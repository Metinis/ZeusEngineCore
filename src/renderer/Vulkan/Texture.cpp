#include "Texture.h"
#include <variant>
#include "Backend/APIRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

using namespace ZEN::VKAPI;

using Pixel = std::array<std::byte, 4>;

Texture::Texture(TextureInfo& texInfo) {
    m_APIRenderer = texInfo.apiRenderer;

    ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.allocator = texInfo.allocator;
    imageCreateInfo.queueFamily = texInfo.queueFamily;
    imageCreateInfo.destroyCallback = texInfo.destroyCallback;

    int texWidth, texHeight, texChannels;
    Bitmap bitmap;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* pixels = stbi_load(texInfo.filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    if(!pixels){
        throw std::runtime_error("Invalid image!");
    }
    size_t byteSize = static_cast<size_t>(texWidth) * texHeight * 4;
    bitmap.bytes = std::span<std::byte const>(reinterpret_cast<std::byte const*>(pixels), byteSize);
    bitmap.size = {texWidth, texHeight};
    bitmap.owner = static_cast<void*>(pixels);

    SampledImage sampledImage(imageCreateInfo, std::move(*texInfo.commandBlock),
                              bitmap);
    stbi_image_free(const_cast<void*>(bitmap.owner));
    m_Image.emplace(std::move(sampledImage));

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    vk::ImageSubresourceRange subresourceRange{};
    subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
    subresourceRange.setLayerCount(1);
    subresourceRange.setLevelCount(m_Image.value().Get().GetLevels());

    imageViewCreateInfo.setImage(m_Image.value().Get().Get());
    imageViewCreateInfo.setViewType(vk::ImageViewType::e2D);
    imageViewCreateInfo.setFormat(m_Image.value().Get().GetFormat());
    imageViewCreateInfo.setSubresourceRange(subresourceRange);

    m_View = texInfo.device.createImageViewUnique(imageViewCreateInfo);
    m_Sampler = texInfo.device.createSamplerUnique(texInfo.sampler);
}

void Texture::Bind() {
    vk::DescriptorImageInfo descImageInfo{};
    descImageInfo.setImageView(*m_View);
    descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
    descImageInfo.setSampler(*m_Sampler);
    m_APIRenderer->SetImage(descImageInfo);
}

Texture::~Texture() = default;



