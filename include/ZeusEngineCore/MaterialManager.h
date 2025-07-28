#pragma once
#include "Material.h"

namespace ZEN {
    class MaterialManager {
    public:
        std::shared_ptr<Material> Load(const std::string &name, std::shared_ptr<IShader> &shader);

        std::shared_ptr<Material> Get(const std::string &name);

    private:
        std::unordered_map<std::string, std::shared_ptr<Material>> m_Materials;

    };
}
