#pragma once
#include <vulkan/vulkan.hpp>
#include <span>
#include <functional>
#include "vma/vk_mem_alloc.h"
#include "../../src/renderer/Vulkan/Backend/VulkanBuffer.h"

class VKRenderer;
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
    std::shared_ptr<std::function<void(BufferHandle)>> deferredDestroyBuffer;
};
struct OpenGLContextInfo {};

using BackendContextVariant = std::variant<std::monostate, VulkanContextInfo, OpenGLContextInfo>;

struct VKShaderVertexInput {
    std::span<vk::VertexInputAttributeDescription2EXT const> attributes{};
    std::span<vk::VertexInputBindingDescription2EXT const> bindings{};
};

struct VulkanShaderInfo {
    vk::Device device;
    std::span<vk::DescriptorSetLayout const> setLayouts;
    VKShaderVertexInput vertexInput{};

};
struct OpenGLShaderInfo {};