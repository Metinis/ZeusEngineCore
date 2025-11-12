#pragma once
#include "ZeusEngineCore/Vertex.h"
#include "ZeusEngineCore/Util.h"
#include "ZeusEngineCore/Entity.h"

namespace entt {
    enum class entity : std::uint32_t;
}

namespace ZEN {
    class Entity;

    struct MeshComp {
        std::string name{};
    };
    struct MeshDrawableComp {
        std::string name{};
        size_t indexCount{};
        uint32_t meshID{};
        int instanceCount{1};
    };
    struct MaterialComp {
        std::string name{};
    };
    struct SkyboxComp {
        MaterialComp skyboxMat{};
        MaterialComp eqMat{};
        MaterialComp conMat{};
        MaterialComp prefilterMat{};
        MaterialComp brdfLUTMat{};
        bool envGenerated{};
    };
    struct UniformComp {
        uint32_t uboID{};
    };
    struct TagComp {
        std::string tag;
    };
    struct TransformComp {
        glm::vec3 localPosition{0.0f, 0.0f, 0.0f};
        glm::vec3 localRotation{0.0f, 0.0f, 0.0f};
        glm::vec3 localScale{1.0f, 1.0f, 1.0f};

        glm::mat4 worldMatrix{1.0f}; //needs to be computed each frame in a render system
        //because it relies on parent

        [[nodiscard]] glm::mat4 getLocalMatrix() const {
            auto const [t, r, s] = toMatrices(localPosition, localRotation, localScale);
            return t * r * s;
        }

        //world position
        [[nodiscard]] glm::mat4 getViewMatrix() const {
            auto const [t, r, s] = toMatrices(-glm::vec3(worldMatrix[3]), -localRotation, localScale);
            return r * t * s;
        }

        [[nodiscard]] glm::vec3 getFront() const {
            float pitch = glm::radians(localRotation.x);
            float yaw   = glm::radians(localRotation.y);

            glm::vec3 front;
            front.x = sin(yaw) * cos(pitch);
            front.y = -sin(pitch);
            front.z = cos(yaw) * cos(pitch);

            return -glm::normalize(front);
        }
    };

    struct DirectionalLightComp {
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
    struct ParentComp {
        Entity parent;
    };

}
