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

class VulkanBackend {
public:
    VulkanBackend(const std::vector<const char*>& layers, WindowHandle windowHandle);
    ~VulkanBackend();
    void Init();
    VulkanContextInfo GetContext();
    VulkanShaderInfo GetShaderInfo() const;
    bool AcquireRenderTarget();
    vk::CommandBuffer BeginFrame();
    void TransitionForRender(vk::CommandBuffer const commandBuffer) const;
    void Render(vk::CommandBuffer const commandBuffer, const std::function<void(vk::CommandBuffer)>& drawCallback = nullptr,
                const std::function<void(vk::CommandBuffer)>& uiDrawCallback = nullptr);
    void TransitionForPresent(vk::CommandBuffer const commandBuffer) const;
    void SubmitAndPresent();
    glm::ivec2 GetFramebufferSize() const {return m_FramebufferSize;}
    std::size_t GetFrameIndex() const { return m_Sync.GetFrameIndex(); }
    VulkanDescriptorBuffer CreateUBO() const;

    void BindDescriptorSets(vk::CommandBuffer const commandBuffer, 
        VulkanDescriptorBuffer& ubo)
    {
        m_DescSet.BindDescriptorSets(commandBuffer, GetFrameIndex(),
            ubo.GetDescriptorInfoAt(GetFrameIndex()));
    }
private:
    std::vector<const char*> GetRequiredExtensions();
    vk::UniqueDebugUtilsMessengerEXT CreateMessenger(const vk::Instance instance);
    vk::UniqueSurfaceKHR CreateSurface(WindowHandle windowHandle, const vk::Instance instance);
    VulkanSwapchain CreateSwapchain(const WindowHandle windowHandle, const vk::Device device,
        const GPU& gpu, const vk::SurfaceKHR surface);
    VulkanMemAlloc CreateMemoryAllocator(const vk::Instance instance, const vk::PhysicalDevice physicalDevice,
                                         const vk::Device logicalDevice) const;
    vk::UniqueCommandPool CreateCommandBlockPool() const;
    void FlushDeferredDestroys();
    //void DestroyBuffersDeffered(VmaAllocator allocator, VmaAllocation allocation, vk::Buffer buffer);

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

    //deferred buffer destruction
    std::shared_ptr<std::function<void(BufferHandle)>> m_DeferredDestroyCallback;
    std::vector<BufferHandle> m_DeferredDestroy;
    
};
