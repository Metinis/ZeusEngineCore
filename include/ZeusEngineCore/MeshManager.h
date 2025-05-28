#pragma once
#include <memory>
#include <unordered_map>

#include "IMesh.h"

class MeshManager {
public:
    static std::shared_ptr<IMesh> Load(const std::string &name, const std::vector<Vertex>& vertices,
        const std::vector<uint32_t>& indices, RendererAPI api);

    static std::shared_ptr<IMesh> Get(const std::string &name);
private:
    static std::unordered_map<std::string, std::shared_ptr<IMesh>> s_Meshes;
};
