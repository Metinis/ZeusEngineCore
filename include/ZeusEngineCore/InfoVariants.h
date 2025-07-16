#pragma once
#include <vulkan/vulkan.hpp>
#include <span>
#include <variant>
#include <functional>
#include "vma/vk_mem_alloc.h"
#include "../../src/renderer/Vulkan/Backend/CommandBlock.h"
#include <cstdint>

namespace ZEN {
    namespace VKAPI{
        struct BufferHandle;
        struct ImageHandle;
        using DeferredHandle = std::variant<BufferHandle, ImageHandle>;

        struct BackendInfo{
            std::uint32_t apiVersion{};
            vk::Instance instance{};
            vk::PhysicalDevice physicalDevice{};
            vk::Device device{};
            std::uint32_t queueFamily{};
            vk::Queue queue{};
            vk::SampleCountFlagBits samples{};
            vk::Format colorFormat{};
        };
    }
    namespace OGLAPI{
        struct BackendInfo{

        };
    }
}