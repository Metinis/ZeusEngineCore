
#include "IResourceManager.h"
#include "OpenGL/GLResourceManager.h"

using namespace ZEN;
std::unique_ptr<IResourceManager> IResourceManager::Create(eRendererAPI api) {
    switch (api) {
        case OpenGL:
            return std::make_unique<GLResourceManager>();
            break;

        case Vulkan:
            //return std::make_unique<VKResourceManager>();
            break;

        default:
            throw std::runtime_error("Invalid API!");
    }
}
