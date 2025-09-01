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
    class GLResourceManager : public IResourceManager{
    public:
        ~GLResourceManager() override;
        uint32_t createMeshDrawable(const MeshComp& meshComp)  override;
        void bindMeshDrawable(uint32_t drawableID);
        void deleteMeshDrawable(uint32_t drawableID) override;
        uint32_t createShader(std::string_view vertexPath, std::string_view fragPath) override;
        void bindShader(uint32_t shaderID) override;
        void deleteShader(uint32_t shaderID) override;
        uint32_t createUBO(uint32_t binding) override;
        void writeToUBO(uint32_t uboID, std::span<const std::byte> bytes) override;
        void bindUBO(uint32_t uboID) override;
        void deleteUBO(uint32_t uboID) override;
    private:
        std::unordered_map<uint32_t, GLShader> m_Shaders{};
        uint32_t nextShaderID{1};
        std::unordered_map<uint32_t, GLDrawable> m_Drawables{};
        uint32_t nextDrawableID{1};
        std::unordered_map<uint32_t, GLUniform> m_UBOs{};
        uint32_t nextUBOID{1};
    };
};

