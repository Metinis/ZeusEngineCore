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
        glm::vec3 position{0.0f, 0.0f, -1.0f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};
    };
    struct CameraComp {
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
        glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 up = glm::vec3(0.0f, 1.0f,  0.0f);

        float aspect = 16.0f / 9.0f;
        float fov = glm::radians(60.0f);
        float near = 0.1f;
        float far = 100.0f;

        bool isPrimary = true;
    };

}