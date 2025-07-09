#pragma once
#include "ZeusEngineCore/ITexture.h"
#include "Backend/VulkanSampledImage.h"



class VKTexture : public ITexture{
public:
	void Init(TextureInfo& textureInfo) override;
    vk::DescriptorImageInfo GetDescriptorInfo() const;
private:
	std::optional<VulkanSampledImage> m_Image;
	vk::UniqueImageView m_View{};
	vk::UniqueSampler m_Sampler{};


};