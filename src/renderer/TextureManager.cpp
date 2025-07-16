#include "ZeusEngineCore/TextureManager.h"
#include "ZeusEngineCore/ITexture.h"

using namespace ZEN;

TextureManager::TextureManager(IRendererBackend* backendAPI, IRendererAPI* rendererAPI)
        : m_BackendAPI(backendAPI),
          m_RendererAPI(rendererAPI)
{

}
std::shared_ptr<ITexture> TextureManager::Load(const std::string &name,
                                              const std::string &filepath) {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end())
        return it->second;

    auto texture = ITexture::Create(m_BackendAPI, m_RendererAPI, filepath);
    m_Textures[name] = texture;
    return texture;
}
std::shared_ptr<ITexture> TextureManager::Get(const std::string &name) {
    auto it = m_Textures.find(name);
    if(it != m_Textures.end())
        return it->second;
    return nullptr;
}