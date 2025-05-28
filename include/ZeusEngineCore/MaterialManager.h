#pragma once
#include "Material.h"

class MaterialManager {
public:
    static std::shared_ptr<Material> Load(const std::string &name, std::shared_ptr<IShader>& shader);
    static std::shared_ptr<Material> Get(const std::string &name);
private:
    static std::unordered_map<std::string, std::shared_ptr<Material>> s_Materials;

};
