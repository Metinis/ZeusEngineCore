#include "IContext.h"
#include "OpenGL/GLContext.h"
#include "ZeusEngineCore/Application.h"

std::unique_ptr<ZEN::IContext> ZEN::IContext::create() {
    switch (Application::get().getRendererAPI()) {
        case OpenGL:
            return std::make_unique<GLContext>();

        case Vulkan:
            return nullptr;
            break;

        default:
            throw std::runtime_error("Invalid API!");
    }
}
