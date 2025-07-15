#pragma once
#include <memory>
#include "../../src/renderer/Vulkan/Backend/APIRenderer.h"
#include "../../src/renderer/OpenGL/Shader.h"

namespace ZEN {
    class ShaderManager {
    public:
        ShaderManager(VKAPI::APIBackend* backendAPI, VKAPI::APIRenderer* rendererAPI);

        std::shared_ptr<IShader> Load(const std::string &name, const std::string &vertexPath,
                                      const std::string &fragmentPath);

        std::shared_ptr<IShader> Get(const std::string &name);

    private:
        std::unordered_map<std::string, std::shared_ptr<IShader>> m_Shaders;

        VKAPI::APIBackend* m_BackendAPI;
        VKAPI::APIRenderer* m_RendererAPI;
    };
}
