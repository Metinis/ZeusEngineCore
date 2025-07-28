#pragma once
#include <memory>
#include <unordered_map>
#include <string>

namespace ZEN {
    class IRendererBackend;
    class IRendererAPI;
    class IShader;

    class ShaderManager {
    public:
        ShaderManager(IRendererBackend* backendAPI, IRendererAPI* rendererAPI);

        std::shared_ptr<IShader> Load(const std::string &name, const std::string &vertexPath,
                                      const std::string &fragmentPath);

        std::shared_ptr<IShader> Get(const std::string &name);

    private:
        std::unordered_map<std::string, std::shared_ptr<IShader>> m_Shaders;

        IRendererBackend* m_BackendAPI;
        IRendererAPI* m_RendererAPI;
    };
}
