#pragma once
#include <vulkan/vulkan.hpp>
#include "VulkanInstance.h"
#include "VulkanDevice.h"
#include "../../../Utils.h"
#include "ZeusEngineCore/ScopedWaiter.h"
#include "VulkanSwapchain.h"
#include "VulkanSync.h"
#include "ZeusEngineCore/InfoVariants.h"
#include <functional>

class VulkanBackend {
public:
    VulkanBackend(const std::vector<const char*>& layers, WindowHandle windowHandle);
    ~VulkanBackend() = default;
    void Init();
    VulkanContextInfo GetContext() const;
    VulkanShaderInfo GetShaderInfo() const;
    bool AcquireRenderTarget();
    vk::CommandBuffer BeginFrame();
    void TransitionForRender(vk::CommandBuffer const commandBuffer) const;
    void Render(vk::CommandBuffer const commandBuffer, const std::function<void(vk::CommandBuffer)>& drawCallback = nullptr,
                const std::function<void(vk::CommandBuffer)>& uiDrawCallback = nullptr);
    void TransitionForPresent(vk::CommandBuffer const commandBuffer) const;
    void SubmitAndPresent();
    glm::ivec2 GetFramebufferSize() const {return m_FramebufferSize;}
private:
    std::vector<const char*> GetRequiredExtensions();
    vk::UniqueDebugUtilsMessengerEXT CreateMessenger(const vk::Instance instance);
    vk::UniqueSurfaceKHR CreateSurface(WindowHandle windowHandle, const vk::Instance instance);
    VulkanSwapchain CreateSwapchain(const WindowHandle windowHandle, const vk::Device device,
        const GPU& gpu, const vk::SurfaceKHR surface);

    //order matters
    VulkanInstance m_Instance;
    WindowHandle m_WindowHandle;
    vk::UniqueSurfaceKHR m_Surface;
    VulkanDevice m_Device;
    vk::UniqueDebugUtilsMessengerEXT m_DebugMessenger;
    ScopedWaiter m_Waiter;
    VulkanSync m_Sync;
    VulkanSwapchain m_Swapchain;

    //rendering backend variables
    glm::ivec2 m_FramebufferSize{};
    std::optional<RenderTarget> m_RenderTarget{};
    
};
