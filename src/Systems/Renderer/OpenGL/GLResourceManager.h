#pragma once
#include <unordered_map>
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
    class GLResourceManager : public IResourceManager{
    public:
        GLResourceManager();
        ~GLResourceManager() override;

        uint32_t createMeshDrawable(const MeshComp& meshComp) override;
        void bindMeshDrawable(uint32_t drawableID) override;
        void deleteMeshDrawable(uint32_t drawableID) override;

        uint32_t createShader(std::string_view vertexPath, std::string_view fragPath) override;
        void bindShader(uint32_t shaderID) override;
        void deleteShader(uint32_t shaderID) override;

        uint32_t createUBO(uint32_t binding) override;
        void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) override;
        void bindUBO(uint32_t uboID) override;
        void deleteUBO(uint32_t uboID) override;

        uint32_t createTexture(std::string_view texturePath) override;
        void bindTexture(uint32_t textureID, uint32_t binding) override;
        void deleteTexture(uint32_t textureID) override;

        uint32_t createCubeMapTexture(const std::string& texturePath)  override;
        void bindCubeMapTexture(uint32_t textureID) override;

        uint32_t createFBO() override;
        void bindFBO(uint32_t fboID) override;

        uint32_t createColorTex(int width, int height) override;
        uint32_t getTexture(uint32_t textureID) override;
        uint32_t createDepthBuffer(int width, int height) override;
        void deleteDepthBuffer(uint32_t bufferID) override;

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

