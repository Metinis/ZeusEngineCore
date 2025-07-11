#pragma once
#include <vulkan/vulkan.hpp>
#include <span>
#include <variant>
#include <functional>
#include "vma/vk_mem_alloc.h"
#include "../../src/renderer/Vulkan/Backend/VulkanCommandBlock.h"
#include <cstdint>

class VKRenderer;
struct BufferHandle;
struct ImageHandle;

using DeferredHandle = std::variant<BufferHandle, ImageHandle>;

struct VulkanContextInfo {
    std::uint32_t apiVersion{};
    vk::Instance instance{};
    vk::PhysicalDevice physicalDevice{};
    std::uint32_t queueFamily{};
    vk::Device device{};
    vk::Queue queue{};
    vk::Format colorFormat{};
    vk::SampleCountFlagBits samples{};
    VmaAllocator allocator{};
    vk::CommandPool commandBlockPool{};
    std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback;
};
struct OpenGLContextInfo {};

using BackendContextVariant = std::variant<std::monostate, VulkanContextInfo, OpenGLContextInfo>;

struct VKShaderVertexInput {
    std::span<vk::VertexInputAttributeDescription2EXT const> attributes{};
    std::span<vk::VertexInputBindingDescription2EXT const> bindings{};
};

struct VulkanShaderInfo {
    vk::Device device{};
    vk::SampleCountFlagBits samples{};
    vk::Format colorFormat{};
    vk::Format depthFormat{};
    vk::PipelineLayout pipelineLayout{};
};
struct OpenGLShaderInfo {};

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

struct VulkanTextureInfo{
    vk::Device device{};
    VmaAllocator allocator{};
    std::uint32_t queueFamily{};
    std::optional<VulkanCommandBlock> commandBlock;
    vk::SamplerCreateInfo sampler{samplerCreateInfo_v};
    std::shared_ptr<std::function<void(DeferredHandle)>> destroyCallback;
};

struct OpenGLTextureInfo {};

using TextureInfoVariant = std::variant<std::monostate, VulkanTextureInfo, OpenGLTextureInfo>;