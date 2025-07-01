#pragma once
#include <vulkan/vulkan.hpp>
#include <span>
#include "vma/vk_mem_alloc.h"

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