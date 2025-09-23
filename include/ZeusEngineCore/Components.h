#pragma once
#include "ZeusEngineCore/Vertex.h"
#include "ZeusEngineCore/Util.h"

namespace ZEN {
    struct Mesh {
        std::vector<uint32_t> indices{};
        std::vector<Vertex> vertices{};
    };
    struct MeshComp {
        std::vector<Mesh> meshes{};
    };
    struct MeshDrawable {
        size_t indexCount{};
        uint32_t meshID{};
        int instanceCount{1};
    };
    struct MeshDrawableComp {
        std::vector<MeshDrawable> drawables{};
    };
    struct MaterialComp {
        uint32_t shaderID{};
        std::vector<uint32_t> textureIDs{};
        std::vector<uint32_t> specularTexIDs{};
        float specular{1.0f};
        int shininess{1};
    };
    struct SkyboxComp {
        uint32_t shaderID;
        uint32_t textureID;
    };
    struct UniformComp {
        uint32_t uboID{};
    };
    struct TagComp {
        std::string tag;
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

        [[nodiscard]] glm::vec3 getFront() const {
            // rotation.x = pitch, rotation.y = yaw, rotation.z = roll
            float pitch = glm::radians(rotation.x);
            float yaw   = glm::radians(rotation.y);

            glm::vec3 front;
            front.x = sin(yaw) * cos(pitch);   // swapped sin/cos
            front.y = -sin(pitch);
            front.z = cos(yaw) * cos(pitch);   // positive Z is forward

            return -glm::normalize(front);
        }
    };
    struct DirectionalLightComp {
        glm::vec3 lightDir{};
        glm::vec3 ambient{};
        bool isPrimary{true};
    };
    struct PointLightComp {
        glm::vec3 ambient{};
        glm::vec3 diffuse{};
        glm::vec3 specular{};

        float constant{};
        float linear{};
        float quadratic{};
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