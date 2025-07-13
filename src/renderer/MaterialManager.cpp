#include "ZeusEngineCore/MaterialManager.h"
#include <unordered_map>
#include "ZeusEngineCore/Material.h"

using namespace ZEN;

std::shared_ptr<Material> MaterialManager::Load(const std::string &name, std::shared_ptr<IShader>& shader) {
    auto it = m_Materials.find(name);
    if(it != m_Materials.end())
        return it->second;

    auto material = std::make_shared<Material>(shader);
    m_Materials[name] = material;
    return material;
}
std::shared_ptr<Material> MaterialManager::Get(const std::string &name) {
    auto it = m_Materials.find(name);
    if(it != m_Materials.end())
        return it->second;
    return nullptr;
}
