#include "Texture.h"
#include <variant>
#include "Backend/APIRenderer.h"

using namespace ZEN::VKAPI;

// 4-channels.
constexpr auto whitePixel_v = std::array{ std::byte{0xff}, std::byte{0xff},
										  std::byte{0xff}, std::byte{0xff} };
// fallback bitmap.
constexpr auto whiteBitmap_v = Bitmap{
  .bytes = whitePixel_v,
  .size = {1, 1},
};
using Pixel = std::array<std::byte, 4>;
static constexpr auto rgby_pixels_v = std::array{
  Pixel{std::byte{0xff}, {}, {}, std::byte{0xff}},
  Pixel{std::byte{}, std::byte{0xff}, {}, std::byte{0xff}},
  Pixel{std::byte{}, {}, std::byte{0xff}, std::byte{0xff}},
  Pixel{std::byte{0xff}, std::byte{0xff}, {}, std::byte{0xff}},
};
static constexpr auto rgby_bytes_v =
std::bit_cast<std::array<std::byte, sizeof(rgby_pixels_v)>>(
	rgby_pixels_v);
static constexpr auto rgby_bitmap_v = Bitmap{
  .bytes = rgby_bytes_v,
  .size = {2, 2},
};
static constexpr auto test_pixels = std::array{
  Pixel{std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0xff}},
  Pixel{std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0xff}},
  Pixel{std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0xff}},
  Pixel{std::byte{0xff}, std::byte{0xff}, std::byte{0xff}, std::byte{0xff}},
};
static constexpr auto test_bytes_v =
std::bit_cast<std::array<std::byte, sizeof(test_pixels)>>(
	test_pixels);

static constexpr auto test_bitmap_v = Bitmap{
  .bytes = test_bytes_v,
  .size = {4, 4},
};

Texture::Texture(TextureInfo& texInfo) {
    m_APIRenderer = texInfo.apiRenderer;

    ImageCreateInfo imageCreateInfo{};
    imageCreateInfo.allocator = texInfo.allocator;
    imageCreateInfo.queueFamily = texInfo.queueFamily;
    imageCreateInfo.destroyCallback = texInfo.destroyCallback;

    Bitmap bitmap;
    bitmap = rgby_bitmap_v; //placeholder
    SampledImage sampledImage(imageCreateInfo, std::move(*texInfo.commandBlock),
                              bitmap);
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

vk::DescriptorImageInfo Texture::GetDescriptorInfo() const
{
	vk::DescriptorImageInfo descImageInfo{};
	descImageInfo.setImageView(*m_View);
	descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	descImageInfo.setSampler(*m_Sampler);
	return descImageInfo;
}

void Texture::Bind() {
    m_APIRenderer->SetImage(*this);
    m_APIRenderer->BindDescriptorSets(); //placeholder
}

Texture::~Texture() = default;



