#pragma once
#include <vulkan/vulkan.hpp>
#include "Instance.h"
#include "Device.h"
#include "../../../Utils.h"
#include "ZeusEngineCore/ScopedWaiter.h"
#include "Swapchain.h"
#include "Sync.h"
#include "ZeusEngineCore/InfoVariants.h"
#include "MemAllocator.h"
#include <functional>
#include "CommandBlock.h"
#include "DescriptorBuffer.h"
#include "DescriptorSet.h"
#include "Image.h"
#include "PipelineBuilder.h"


namespace ZEN::VKAPI {
    class Backend {
    public:
        Backend(WindowHandle windowHandle);

        ~Backend();

        void Init();

        void TransitionForRender(vk::CommandBuffer commandBuffer) const;

        void
        Render(vk::CommandBuffer commandBuffer,
               const std::function<void(vk::CommandBuffer)> &drawCallback = nullptr,
               const std::function<void(vk::CommandBuffer)> &uiDrawCallback = nullptr);

        void TransitionForPresent(vk::CommandBuffer commandBuffer) const;

        void SubmitAndPresent();

        bool AcquireRenderTarget();

        void BindDescriptorSets(vk::CommandBuffer const commandBuffer,
                                DescriptorBuffer &ubo, const vk::DescriptorImageInfo &descriptorImageInfo) {
            m_DescSet.BindDescriptorSets(commandBuffer, GetFrameIndex(),
                                         ubo.GetDescriptorInfoAt(GetFrameIndex()), descriptorImageInfo);
        }

        ContextInfo GetContext();

        [[nodiscard]] ShaderInfo GetShaderInfo() const;

        TextureInfo GetTextureInfo();

        [[nodiscard]] vk::Extent2D GetExtent() const { return m_RenderTarget->extent; }

        vk::CommandBuffer BeginFrame();

        [[nodiscard]] glm::ivec2 GetFramebufferSize() const { return m_FramebufferSize; }

        [[nodiscard]] std::size_t GetFrameIndex() const { return m_Sync.GetFrameIndex(); }

        [[nodiscard]] DescriptorBuffer CreateUBO() const;

    private:
        //order matters
        Instance m_Instance;
        WindowHandle m_WindowHandle;
        vk::UniqueSurfaceKHR m_Surface;
        Device m_Device;
        vk::UniqueDebugUtilsMessengerEXT m_DebugMessenger;
        ScopedWaiter m_Waiter;
        Sync m_Sync;
        Swapchain m_Swapchain;
        MemAllocator m_Allocator;
        vk::UniqueCommandPool m_CommandBlockPool;
        DescriptorSet m_DescSet;

        //rendering backend variables
        glm::ivec2 m_FramebufferSize{};
        std::optional<RenderTarget> m_RenderTarget{};

        //deferred resource destruction
        using DeferredHandle = std::variant<BufferHandle, ImageHandle>;
        std::shared_ptr<std::function<void(DeferredHandle)>> m_DeferredDestroyCallback;
        std::vector<DeferredHandle> m_DeferredDestroy;

        void FlushDeferredDestroys();

        vk::UniqueDebugUtilsMessengerEXT CreateMessenger(vk::Instance instance);

        vk::UniqueSurfaceKHR CreateSurface(WindowHandle windowHandle, vk::Instance instance);

        Swapchain CreateSwapchain(WindowHandle windowHandle, vk::Device device,
                                  const GPU &gpu, vk::SurfaceKHR surface);

        [[nodiscard]] MemAllocator CreateMemoryAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice,
                                                         vk::Device logicalDevice) const;

        [[nodiscard]] vk::UniqueCommandPool CreateCommandBlockPool() const;

    };
}
