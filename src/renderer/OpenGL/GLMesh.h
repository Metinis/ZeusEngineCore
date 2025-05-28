
#pragma once
#include "ZeusEngineCore/IMesh.h"

class GLMesh : public IMesh{
public:
    void Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) override;
    ~GLMesh() override;
    void Draw(Material& material) const override;
private:
    uint32_t m_VAO, m_VBO, m_EBO;
    uint32_t m_IndexCount;
};
