#pragma once
#include <cstdint>
#include <ZeusEngineCore/API.h>
#include "ZeusEngineCore/Components.h"
#include <memory>
#include <unordered_map>
#include <iostream>

namespace ZEN {
    class IResourceManager {
    public:
        template<typename T, typename F>
        static void withResource(std::unordered_map<uint32_t, T>& resources, uint32_t id, F&& func) {
            auto it = resources.find(id);
            if (it == resources.end()) {
                std::cerr << "Resource not found: " << id << "\n";
                return;
            }
            func(it->second);
        }

        virtual ~IResourceManager() = default;

        virtual uint32_t createMeshDrawable(const MeshComp& meshComp) = 0;
        virtual void bindMeshDrawable(uint32_t drawableID) = 0;
        virtual void deleteMeshDrawable(uint32_t drawableID) = 0;

        virtual uint32_t createShader(std::string_view vertexPath, std::string_view fragPath) = 0;
        virtual void bindShader(uint32_t shaderID) = 0;
        virtual void deleteShader(uint32_t shaderID) = 0;

        virtual uint32_t createUBO(uint32_t binding) = 0;
        virtual void bindUBO(uint32_t uboID) = 0;
        virtual void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) = 0;
        virtual void deleteUBO(uint32_t uboID) = 0;

        virtual uint32_t createTexture(std::string_view texturePath) = 0;
        virtual void bindTexture(uint32_t textureID, uint32_t binding) = 0;
        virtual void deleteTexture(uint32_t textureID) = 0;

        virtual uint32_t createCubeMapTexture(const std::string& texturePath) = 0;
        virtual void bindCubeMapTexture(uint32_t textureID) = 0;

        virtual uint32_t createFBO() = 0;
        virtual void bindFBO(uint32_t fboID) = 0;

        virtual uint32_t createColorTex(int width, int height) = 0;
        virtual uint32_t getTexture(uint32_t textureID) = 0;
        virtual uint32_t createDepthBuffer(int width, int height) = 0;
        virtual void deleteDepthBuffer(uint32_t bufferID) = 0;

        static std::unique_ptr<IResourceManager> create(eRendererAPI api);
    };
}
