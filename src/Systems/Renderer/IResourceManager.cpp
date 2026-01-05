#include "IResourceManager.h"
#include "OpenGL/GLResourceManager.h"
#include "ZeusEngineCore/core/Application.h"

using namespace ZEN;
std::unique_ptr<IResourceManager> IResourceManager::create() {
    switch (Application::get().getRendererAPI()) {
        case OpenGL:
            return std::make_unique<GLResourceManager>();

        case Vulkan:
            return nullptr;
            //return std::make_unique<VKResourceManager>();

        default:
            throw std::runtime_error("Invalid API!");
    }
}
