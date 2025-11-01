#pragma once
#include <cstdint>
#include <ZeusEngineCore/API.h>
#include <memory>
#include <iostream>
#include <assimp/scene.h>
#include <unordered_map>
#include <span>

namespace ZEN {
    struct Mesh;
    struct Material;
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

        virtual uint32_t createMeshDrawable(const Mesh& mesh) = 0;
        virtual void bindMeshDrawable(uint32_t drawableID) = 0;
        virtual void deleteMeshDrawable(uint32_t drawableID) = 0;

        virtual uint32_t createShader(const std::string& vertexPath, const std::string& ragPath) = 0;
        virtual void bindShader(uint32_t shaderID) = 0;
        virtual void deleteShader(uint32_t shaderID) = 0;

        virtual uint32_t createUBO(uint32_t binding) = 0;
        virtual void bindUBO(uint32_t uboID) = 0;
        virtual void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) = 0;
        virtual void deleteUBO(uint32_t uboID) = 0;

        virtual uint32_t createTexture(const std::string& texturePath, bool isAbsPath) = 0;
        virtual uint32_t createHDRTexture(const std::string& texturePath) = 0;
        virtual uint32_t createTextureAssimp(const aiTexture& aiTex) = 0;
        virtual uint32_t createBRDFLUTTexture(uint32_t width, uint32_t height) = 0;
        virtual void genMipMapCubeMap(uint32_t textureID) = 0;
        virtual void setFBOCubeMapTexture(uint32_t binding, uint32_t textureID, unsigned int mip) = 0;
        virtual void setFBOTexture2D(uint32_t binding, uint32_t textureID, unsigned int mip) = 0;
        virtual void bindTexture(uint32_t textureID, uint32_t binding) = 0;
        virtual void deleteTexture(uint32_t textureID) = 0;

        virtual void bindMaterial(const Material& material) = 0;

        virtual uint32_t createCubeMapTexture(const std::string& texturePath) = 0;
        virtual uint32_t createCubeMapTextureHDRMip(uint32_t width, uint32_t height) = 0;
        virtual uint32_t createCubeMapTextureHDR(uint32_t width, uint32_t height) = 0;
        virtual uint32_t createPrefilterMap(uint32_t width, uint32_t height) = 0;
        virtual void bindCubeMapTexture(uint32_t textureID, uint32_t binding) = 0;

        virtual uint32_t createFBO() = 0;
        virtual void bindFBO(uint32_t fboID) = 0;

        virtual uint32_t createColorTex(int width, int height) = 0;
        virtual uint32_t getTexture(uint32_t textureID) = 0;
        virtual uint32_t createDepthStencilBuffer(int width, int height) = 0;
        virtual uint32_t createDepthBuffer(int width, int height) = 0;
        virtual void updateDepthBufferDimensions(int width, int height) = 0;
        virtual void bindDepthBuffer(uint32_t bufferID) = 0;
        virtual void deleteDepthBuffer(uint32_t bufferID) = 0;

        virtual void pushFloat(uint32_t shaderID, const std::string& name, float value) = 0;

        std::string fullPath(const std::string& path) {
            return m_ResourceRoot + path;
        }

        static std::unique_ptr<IResourceManager> create(eRendererAPI api, const std::string& resourceRoot);
    protected:
        std::string m_ResourceRoot{};
    };
}
