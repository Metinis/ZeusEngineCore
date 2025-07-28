#pragma once
#include <memory>
#include <unordered_map>
#include <string>

namespace ZEN {
    class IRendererBackend;
    class IRendererAPI;
    class ITexture;
    class TextureManager {
    public:
        TextureManager(IRendererBackend* backendAPI, IRendererAPI* rendererAPI);

        std::shared_ptr<ITexture> Load(const std::string &name, const std::string &filepath);

        std::shared_ptr<ITexture> Get(const std::string &name);

    private:
        std::unordered_map<std::string, std::shared_ptr<ITexture>> m_Textures;

        IRendererBackend* m_BackendAPI;
        IRendererAPI* m_RendererAPI;
    };
}