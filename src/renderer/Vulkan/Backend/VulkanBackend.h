#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "../../../Utils.h"
#include "ZeusEngineCore/ScopedWaiter.h"
#include "VulkanSwapchain.h"
#include "VulkanSync.h"
#include "ZeusEngineCore/InfoVariants.h"
#include "VulkanMemAlloc.h"
#include <functional>
#include "VulkanCommandBlock.h"
#include "VulkanDescriptorBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanImage.h"
#include "VulkanPipelineBuilder.h"

class VulkanBackend {
public:
    VulkanBackend(const std::vector<const char*>& layers, WindowHandle windowHandle);
    ~VulkanBackend();
    void Init();
    void TransitionForRender(vk::CommandBuffer commandBuffer) const;
    void Render(vk::CommandBuffer commandBuffer, const std::function<void(vk::CommandBuffer)>& drawCallback = nullptr,
                const std::function<void(vk::CommandBuffer)>& uiDrawCallback = nullptr);
    void TransitionForPresent(vk::CommandBuffer commandBuffer) const;
    void SubmitAndPresent();
    bool AcquireRenderTarget();
    void BindDescriptorSets(vk::CommandBuffer const commandBuffer,
                            VulkanDescriptorBuffer& ubo, const vk::DescriptorImageInfo& descriptorImageInfo)
    {
        m_DescSet.BindDescriptorSets(commandBuffer, GetFrameIndex(),
                                     ubo.GetDescriptorInfoAt(GetFrameIndex()), descriptorImageInfo);
    }
    VulkanContextInfo GetContext();

    [[nodiscard]] VulkanShaderInfo GetShaderInfo() const;
    VulkanTextureInfo GetTextureInfo();
    [[nodiscard]] vk::Extent2D GetExtent() const {return m_RenderTarget->extent;}
    vk::CommandBuffer BeginFrame();
    [[nodiscard]] glm::ivec2 GetFramebufferSize() const {return m_FramebufferSize;}
    [[nodiscard]] std::size_t GetFrameIndex() const { return m_Sync.GetFrameIndex(); }
    [[nodiscard]] VulkanDescriptorBuffer CreateUBO() const;

private:
    //order matters
    VulkanInstance m_Instance;
    WindowHandle m_WindowHandle;
    vk::UniqueSurfaceKHR m_Surface;
    VulkanDevice m_Device;
    vk::UniqueDebugUtilsMessengerEXT m_DebugMessenger;
    ScopedWaiter m_Waiter;
    VulkanSync m_Sync;
    VulkanSwapchain m_Swapchain;
    VulkanMemAlloc m_Allocator;
    vk::UniqueCommandPool m_CommandBlockPool;
    VulkanDescriptorSet m_DescSet;

    //rendering backend variables
    glm::ivec2 m_FramebufferSize{};
    std::optional<RenderTarget> m_RenderTarget{};

    //deferred resource destruction
    using DeferredHandle = std::variant<BufferHandle, ImageHandle>;
    std::shared_ptr<std::function<void(DeferredHandle)>> m_DeferredDestroyCallback;
    std::vector<DeferredHandle> m_DeferredDestroy;

    void FlushDeferredDestroys();
    std::vector<const char*> GetRequiredExtensions();
    vk::UniqueDebugUtilsMessengerEXT CreateMessenger(vk::Instance instance);
    vk::UniqueSurfaceKHR CreateSurface(WindowHandle windowHandle, vk::Instance instance);
    VulkanSwapchain CreateSwapchain(WindowHandle windowHandle, vk::Device device,
        const GPU& gpu, vk::SurfaceKHR surface);
    [[nodiscard]] VulkanMemAlloc CreateMemoryAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice,
                                         vk::Device logicalDevice) const;
    [[nodiscard]] vk::UniqueCommandPool CreateCommandBlockPool() const;

    
};
