#pragma once
#include "vulkan/vulkan.hpp"
#include <optional>
#include "ZeusEngineCore/Utils.h"

namespace ZEN::VKAPI {
    struct GPU {
        vk::PhysicalDevice device{};
        vk::PhysicalDeviceProperties properties{};
        vk::PhysicalDeviceFeatures features{};
        std::uint32_t queueFamily = UINT32_MAX;;
    };

    class Device {
    public:
        Device(vk::Instance instance, vk::SurfaceKHR surface);

        [[nodiscard]] vk::Device GetLogicalDevice() const { return *m_LogicalDevice; };

        [[nodiscard]] const GPU &GetGPU() const { return m_GPU; };

        void SubmitToQueue(vk::SubmitInfo2 submitInfo, vk::Fence drawn);

        [[nodiscard]] vk::Queue GetQueue() const { return m_Queue; }

    private:
        GPU m_GPU;
        vk::UniqueDevice m_LogicalDevice;
        vk::Queue m_Queue;

        vk::UniqueDevice CreateLogicalDevice(const GPU &gpu, vk::Instance instance);

        static GPU FindSuitableGpu(vk::Instance instance, vk::SurfaceKHR surface);
    };
}