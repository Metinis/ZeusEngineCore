#include "ZeusEngineCore/MaterialManager.h"
#include <unordered_map>
#include "ZeusEngineCore/Material.h"

std::unordered_map<std::string, std::shared_ptr<Material>> MaterialManager::s_Materials;

std::shared_ptr<Material> MaterialManager::Load(const std::string &name, std::shared_ptr<IShader>& shader) {
    auto it = s_Materials.find(name);
    if(it != s_Materials.end())
        return it->second;

    auto material = std::make_shared<Material>(shader);
    s_Materials[name] = material;
    return material;
}
std::shared_ptr<Material> MaterialManager::Get(const std::string &name) {
    auto it = s_Materials.find(name);
    if(it != s_Materials.end())
        return it->second;
    return nullptr;
}
