#pragma once
#include "ZeusEngineCore/Vertex.h"
#include "ZeusEngineCore/Util.h"

namespace ZEN {
    struct MeshComp {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };
    struct MeshDrawableComp {
        size_t indexCount;
        uint32_t meshID;
    };
    struct MaterialComp {
        uint32_t shaderID;
        uint32_t textureID;
    };
    struct UniformComp {
        uint32_t uboID;
    };
    struct TransformComp {
        glm::vec3 position{0.0f, 0.0f, 3.0f};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] glm::mat4 getModelMatrix() const {
            auto const [t, r, s] = toMatrices(position, rotation, scale);
            return t * r * s;
        }

        [[nodiscard]] glm::mat4 getViewMatrix() const {
            auto const [t, r, s] = toMatrices(-position, -rotation, scale);
            return r * t * s;
        }
    };
    struct CameraComp {
        glm::mat4 projection = glm::mat4(1.0f);

        float aspect = 16.0f / 9.0f;
        float fov = glm::radians(60.0f);
        float near = 0.1f;
        float far = 100.0f;

        bool isPrimary = true;
    };

}