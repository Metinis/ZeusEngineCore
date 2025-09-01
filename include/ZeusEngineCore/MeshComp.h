#pragma once
#include "ZeusEngineCore/Vertex.h"

namespace ZEN {
    struct MeshComp {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };
    struct MeshDrawableComp {
        size_t indexCount;
        uint32_t meshID;
    };
    struct ShaderComp {
        uint32_t shaderID;
    };
    struct UniformComp {
        uint32_t uboID;
    };
    struct TransformComp {
        glm::vec3 position{};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
    };
    struct CameraComp {
        glm::mat4 view{};
        glm::mat4 projection{};
        bool isPrimary{};
    };
}