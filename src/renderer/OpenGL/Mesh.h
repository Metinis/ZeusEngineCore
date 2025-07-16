
#pragma once
#include "ZeusEngineCore/IMesh.h"

namespace ZEN::OGLAPI{
    class APIRenderer;
    struct MeshInfo{
        APIRenderer* apiRenderer;
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;
    };
    class Mesh : public IMesh {
    public:
        explicit Mesh(const MeshInfo& meshInfo);
        ~Mesh() override;
        void Draw() const override;

    private:
        uint32_t m_VAO, m_VBO, m_EBO;
        uint32_t m_IndexCount;
    };
}

