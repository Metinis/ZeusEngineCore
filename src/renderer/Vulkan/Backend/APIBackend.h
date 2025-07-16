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
#include "../ShaderPipeline.h"
#include "ZeusEngineCore/IRendererBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"

namespace ZEN::VKAPI {
    //Holds all the handles and manages their initialization, hence responsible for returning infos with the
    // required handles
    struct RenderFrameInfo { //info passed to APIRenderer each frame for manipulating accordingly
        glm::ivec2 framebufferSize;
        Sync* sync;
        Device* device;
        Swapchain* swapchain;
    };
    struct MeshInfo;
    struct TextureInfo;
    class APIBackend : public IRendererBackend {
    public:
        APIBackend(WindowHandle windowHandle);

        void FlushDeferredDestroys();

        ~APIBackend();

        void Init();

        ZEN::eRendererAPI GetAPI() const {return ZEN::eRendererAPI::Vulkan;}
        BackendInfo GetInfo() const;
        [[nodiscard]] RenderFrameInfo GetRenderFrameInfo();
        [[nodiscard]] ShaderInfo GetShaderInfo() const;
        MeshInfo GetMeshInfo() const;
        TextureInfo GetTextureInfo();
        DescriptorSet& GetDescriptorSet() {return m_DescSet;}
        [[nodiscard]] glm::ivec2 GetFramebufferSize() const;
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

        //deferred resource destruction
        using DeferredHandle = std::variant<BufferHandle, ImageHandle>;
        std::shared_ptr<std::function<void(DeferredHandle)>> m_DeferredDestroyCallback;
        std::vector<DeferredHandle> m_DeferredDestroy;



        vk::UniqueDebugUtilsMessengerEXT CreateMessenger(vk::Instance instance);

        vk::UniqueSurfaceKHR CreateSurface(WindowHandle windowHandle, vk::Instance instance);

        Swapchain CreateSwapchain(WindowHandle windowHandle, vk::Device device,
                                  const GPU &gpu, vk::SurfaceKHR surface);

        [[nodiscard]] MemAllocator CreateMemoryAllocator(vk::Instance instance, vk::PhysicalDevice physicalDevice,
                                                         vk::Device logicalDevice) const;

        [[nodiscard]] vk::UniqueCommandPool CreateCommandBlockPool() const;

    };
}
