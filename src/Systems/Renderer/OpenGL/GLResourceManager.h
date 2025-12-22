#pragma once

#include "../IResourceManager.h"

namespace ZEN {
    struct GLShader {
        uint32_t programID;
    };
    struct GLDrawable {
        uint32_t vao;
        uint32_t vbo;
        uint32_t ebo;
    };
    struct GLUniform {
        uint32_t bufferHandle;
        uint32_t bufferBinding;
        size_t size;
    };
    struct GLTexture {
        uint32_t textureID;
    };
    struct GLFBO {
        uint32_t handle;
    };
    struct GLDepthBuffer {
        uint32_t handle;
    };

    class GLResourceManager : public IResourceManager {
    public:
        GLResourceManager();
        ~GLResourceManager() override;

        uint32_t createMeshDrawable(const MeshData& mesh) override;
        void bindMeshDrawable(uint32_t drawableID) override;
        void deleteMeshDrawable(uint32_t drawableID) override;

        uint32_t createShader(const std::string& vertexPath, const std::string& fragPath,
            const std::string& geoPath) override;
        void bindShader(uint32_t shaderID) override;
        void deleteShader(uint32_t shaderID) override;

        uint32_t createUBO(uint32_t binding) override;
        void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) override;
        void bindUBO(uint32_t uboID) override;
        void deleteUBO(uint32_t uboID) override;

        uint32_t createTexture(const std::string& texturePath, bool isAbsPath) override;
        uint32_t createHDRTexture(const std::string& texturePath) override;
        uint32_t createTextureAssimp(const aiTexture& aiTex) override;
        uint32_t createBRDFLUTTexture(uint32_t width, uint32_t height) override;
        void genMipMapCubeMap(uint32_t textureID) override;
        void bindTexture(uint32_t textureID, uint32_t binding) override;
        void deleteTexture(uint32_t textureID) override;

        void bindMaterial(const MaterialRaw& material) override;

        uint32_t createCubeMapTexture(const std::string& texturePath) override;
        uint32_t createCubeMapTextureHDRMip(uint32_t width, uint32_t height) override;
        uint32_t createCubeMapTextureHDR(uint32_t width, uint32_t height) override;
        uint32_t createPrefilterMap(uint32_t width, uint32_t height) override;
        void setFBOCubeMapTexture(uint32_t binding, uint32_t textureID, unsigned int mip) override;
        void setFBOTexture2D(uint32_t binding, uint32_t textureID, unsigned int mip) override;
        void bindCubeMapTexture(uint32_t textureID, uint32_t binding) override;

        uint32_t createFBO() override;
        void bindFBO(uint32_t fboID) override;

        uint32_t createColorTex(int width, int height) override;
        uint32_t getTexture(uint32_t textureID) override;
        uint32_t createDepthStencilBuffer(int width, int height) override;
        uint32_t createDepthBuffer(int width, int height) override;
        void updateDepthBufferDimensions(int width, int height) override;
        void bindDepthBuffer(uint32_t bufferID) override;
        void deleteDepthBuffer(uint32_t bufferID) override;

        void pushFloat(uint32_t shaderID, const std::string& name, float value) override;


    private:
        std::unordered_map<uint32_t, GLShader> m_Shaders{};
        uint32_t nextShaderID{1};

        std::unordered_map<uint32_t, GLDrawable> m_Drawables{};
        uint32_t nextDrawableID{1};

        std::unordered_map<uint32_t, GLUniform> m_UBOs{};
        uint32_t nextUBOID{1};

        std::unordered_map<uint32_t, GLTexture> m_Textures{};
        uint32_t nextTextureID{1};

        std::unordered_map<uint32_t, GLFBO> m_FBOs{};
        uint32_t nextFBOID{1};

        std::unordered_map<uint32_t, GLDepthBuffer> m_DepthBuffers{};
        uint32_t nextDepthBufferID{1};
    };

};

