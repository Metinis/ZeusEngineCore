#include "VKTexture.h"
#include <variant>


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


void VKTexture::Init(TextureInfo& textureInfo)
{
	if (!std::holds_alternative<VulkanTextureInfo>(textureInfo.textInfoVariant)) {
		throw std::runtime_error{ "Invalid Texture Info Data For Vulkan!" };
	}
	auto& vkTextureInfo = std::get<VulkanTextureInfo>(textureInfo.textInfoVariant);

	ImageCreateInfo imageCreateInfo{};
	imageCreateInfo.allocator = vkTextureInfo.allocator;
	imageCreateInfo.queueFamily = vkTextureInfo.queueFamily;
	imageCreateInfo.destroyCallback = vkTextureInfo.destroyCallback;

	Bitmap bitmap; 
	bitmap = rgby_bitmap_v; //placeholder
	VulkanSampledImage sampledImage(imageCreateInfo, std::move(vkTextureInfo.commandBlock.value()),
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

	m_View = vkTextureInfo.device.createImageViewUnique(imageViewCreateInfo);
	m_Sampler = vkTextureInfo.device.createSamplerUnique(vkTextureInfo.sampler);
}

vk::DescriptorImageInfo VKTexture::GetDescriptorInfo() const
{
	vk::DescriptorImageInfo descImageInfo{};
	descImageInfo.setImageView(*m_View);
	descImageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	descImageInfo.setSampler(*m_Sampler);
	return descImageInfo;
}

