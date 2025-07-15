#pragma once
#include <memory>
#include <unordered_map>
#include "../../src/renderer/Vulkan/Backend/APIRenderer.h"
#include "IMesh.h"
namespace ZEN {
    class MeshManager {
    public:
        MeshManager(BackendContextVariant backendData, VKAPI::APIRenderer* rendererAPI);

        std::shared_ptr<IMesh> Load(const std::string &name, const std::vector<Vertex> &vertices,
                                    const std::vector<uint32_t> &indices, RendererAPI api);

        std::shared_ptr<IMesh> Get(const std::string &name);

    private:
        std::unordered_map<std::string, std::shared_ptr<IMesh>> m_Meshes;
        BackendContextVariant m_Context;
        VKAPI::APIRenderer* m_RendererAPI;
    };
}
