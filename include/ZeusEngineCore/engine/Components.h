#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/matrix_decompose.hpp"
#include "glm/gtx/quaternion.hpp"
#include "Jolt/Core/Reference.h"
#include "Jolt/Physics/Body/BodyID.h"
#include "Jolt/Physics/Body/MotionType.h"
#include "ZeusEngineCore/core/Vertex.h"
#include "ZeusEngineCore/core/Util.h"
#include "ZeusEngineCore/engine/UUID.h"
#include "ZeusEngineCore/asset/AssetHandle.h"

namespace JPH {
    class Shape;
}

namespace entt {
    enum class entity : std::uint32_t;
}

namespace ZEN {
    class Entity;

    struct UUIDComp {
        UUID uuid;
    };
    struct MeshComp {
        AssetHandle<MeshData> handle;
    };
    struct MeshDrawableComp {
        AssetHandle<GPUMesh> handle;
    };
    struct MaterialComp {
        AssetHandle<Material> handle;
    };
    struct SkyboxComp {
        MaterialComp skyboxMat{};
        MaterialComp eqMat{};
        MaterialComp conMat{};
        MaterialComp prefilterMat{};
        MaterialComp brdfLUTMat{};
        bool envGenerated{};
    };

    struct TagComp {
        std::string tag;
    };
    struct TransformComp {
        glm::vec3 localPosition{0.0f, 0.0f, 0.0f};
        glm::quat localRotation{1.0f, 0.0f, 0.0f, 0.0f};
        glm::vec3 localScale{1.0f, 1.0f, 1.0f};

        glm::mat4 worldMatrix{1.0f}; //needs to be computed each frame in a render system
        //because it relies on parent

        [[nodiscard]] glm::mat4 getLocalMatrix() const {
            glm::mat4 t = glm::translate(glm::mat4(1.0f), localPosition);
            glm::mat4 r = glm::toMat4(localRotation);
            glm::mat4 s = glm::scale(glm::mat4(1.0f), localScale);
            return t * r * s;
        }

        glm::mat4 getViewMatrix() const {
            glm::mat4 rot = glm::mat4_cast(glm::conjugate(localRotation)); // camera looks along -Z
            glm::mat4 trans = glm::translate(glm::mat4(1.0f), -localPosition);
            return rot * trans;
        }

        [[nodiscard]] glm::vec3 getFront() const {
            return glm::normalize(localRotation * glm::vec3(0, 0, -1));
        }
        void decomposeTransform(const glm::mat4& transform)
        {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::quat orientation;

            glm::decompose(transform, localScale, orientation, localPosition, skew, perspective);

            localRotation = orientation;
        }

    };
    struct RigidBodyComp {
        JPH::EMotionType motionType = JPH::EMotionType::Dynamic;

        float mass = 1.0f;
        float linearDamping = 0.0f;
        float angularDamping = 0.05f;

        bool allowSleep = true;
        bool lockPosX = false;
        bool lockPosY = false;
        bool lockPosZ = false;

        bool lockRotX = false;
        bool lockRotY = false;
        bool lockRotZ = false;
    };
    struct PhysicsBodyComp { //runtime only
        JPH::BodyID bodyID = JPH::BodyID();
    };
    struct BoxColliderComp {
        glm::vec3 halfExtents {0.5f};
        glm::vec3 offset {0.0f};
        bool isTrigger = false;
    };
    struct SphereColliderComp {
        float radius = 0.5f;
        glm::vec3 offset {0.0f};
        bool isTrigger = false;
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
    struct SceneCameraComp {
        glm::mat4 projection = glm::mat4(1.0f);

        float aspect = 16.0f / 9.0f;
        float fov = glm::radians(60.0f);
        float near = 0.1f;
        float far = 100.0f;
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
        UUID parentID{};
    };

}
