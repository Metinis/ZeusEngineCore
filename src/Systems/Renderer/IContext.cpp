
#include "IContext.h"

#include "OpenGL/GLContext.h"

std::unique_ptr<ZEN::IContext> ZEN::IContext::create(eRendererAPI api,
    GLFWwindow* window, IResourceManager* resourceManager) {
    switch (api) {
        case OpenGL:
            return std::make_unique<GLContext>(window, resourceManager);

        case Vulkan:
            return nullptr;
            break;

        default:
            throw std::runtime_error("Invalid API!");
    }
}
