#include "ZeusEngineCore/IDescriptorBuffer.h"
#include "ZeusEngineCore/IRendererBackend.h"
#include "ZeusEngineCore/IRendererAPI.h"
#include "OpenGL/APIBackend.h"
#include "OpenGL/APIRenderer.h"
#include "Vulkan/Backend/APIBackend.h"
#include "Vulkan/Backend/APIRenderer.h"
#include "OpenGL/DescriptorBuffer.h"
#include "Vulkan/DescriptorBuffer.h"

using namespace ZEN;

std::unique_ptr<IDescriptorBuffer> IDescriptorBuffer::Create(IRendererBackend* apiBackend,
                                                             IRendererAPI* apiRenderer,
                                                             eDescriptorBufferType type) {
    switch(apiBackend->GetAPI()) {
        case eRendererAPI::OpenGL:{
            auto backendAPI = dynamic_cast<OGLAPI::APIBackend*>(apiBackend);
            auto rendererAPI = dynamic_cast<OGLAPI::APIRenderer*>(apiRenderer);
            OGLAPI::BufferCreateInfo info = backendAPI->GetBufferCreateInfo(type);
            info.apiRenderer = rendererAPI;
            return std::make_unique<OGLAPI::DescriptorBuffer>(info);
        }
        case eRendererAPI::Vulkan:
        {
            auto backendAPI = dynamic_cast<VKAPI::APIBackend*>(apiBackend);
            auto rendererAPI = dynamic_cast<VKAPI::APIRenderer*>(apiRenderer);
            VKAPI::BufferCreateInfo info = backendAPI->GetBufferCreateInfo(type);
            info.apiRenderer = rendererAPI;
            return std::make_unique<VKAPI::DescriptorBuffer>(info);
        }
        default: return nullptr;
    }
}