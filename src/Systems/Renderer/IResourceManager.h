#pragma once
#include <cstdint>
#include <ZeusEngineCore/API.h>
#include "ZeusEngineCore/MeshComp.h"
#include <memory>

namespace ZEN {
    class IResourceManager {
    public:
        virtual ~IResourceManager() = default;
        virtual uint32_t createMeshDrawable(const MeshComp& meshComp) = 0;
        virtual void deleteMeshDrawable(uint32_t drawableID) = 0;
        virtual uint32_t createShader(std::string_view vertexPath, std::string_view fragPath) = 0;
        virtual void bindShader(uint32_t shaderID) = 0;
        virtual void deleteShader(uint32_t shaderID) = 0;
        virtual uint32_t createUBO(uint32_t binding) = 0;
        virtual void bindUBO(uint32_t uboID) = 0;
        virtual void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) = 0;
        virtual void deleteUBO(uint32_t uboID) = 0;


        static std::unique_ptr<IResourceManager> Create(eRendererAPI api);
    };
}
