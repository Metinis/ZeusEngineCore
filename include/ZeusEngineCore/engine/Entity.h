#pragma once
#include <entt/entt.hpp>
#include <utility>
#include "Components.h"
#include "ZeusEngineCore/scripting/CompRegistry.h"
#include <Jolt/Physics/Collision/Shape/Shape.h>

namespace JPH {
    class BodyCreationSettings;
}

namespace ZEN {
    class Scene;
    template<typename T>
        concept IsColliderComp = std::same_as<T, BoxColliderComp> || std::same_as<T, SphereColliderComp>;
    class Entity {
    public:
        explicit Entity(Scene* scene, entt::entity handle);
        //explicit Entity(entt::registry* registry, entt::entity handle);
        explicit Entity() = default;

        template<typename T>
        T& getComponent() {
            return m_Registry->get<T>(m_Handle);
        }

        template<typename T>
        bool hasComponent() {
            return m_Registry->any_of<T>(m_Handle);
        }

        template<typename T, typename... Args>
        requires (!IsColliderComp<T>)
        T& addComponent(Args... args) {
            return m_Registry->emplace<T>(m_Handle,
                std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        requires (IsColliderComp<T>)
        T& addComponent(Args... args) {
            return m_Registry->emplace<T>(m_Handle,
                std::forward<Args>(args)...);
        }

        template<typename T>
        requires (IsColliderComp<T>)
        T& addComponent() {
            if (auto* mesh = tryGetComponent<MeshComp>()) {
                auto& tc = getComponent<TransformComp>();
                glm::vec3 center{};
                if constexpr (std::same_as<T, BoxColliderComp>) {
                    BoxColliderComp comp {
                        .halfExtents = mesh->handle->getHalfExtents(center) * tc.localScale,
                        .offset = center,
                    };
                    return addComponent<T>(comp);
                }
                if constexpr (std::same_as<T, SphereColliderComp>) {
                    float scaleFactor = glm::max(tc.localScale.x, glm::max(tc.localScale.y, tc.localScale.z));
                    SphereColliderComp comp {
                        .radius = mesh->handle->getRadius(center) * scaleFactor,
                        .offset = center,
                    };
                    return addComponent<T>(comp);
                }
            }
            return m_Registry->emplace<T>(m_Handle);
        }

        ParentComp& addParent(const ParentComp &pc);
        ParentComp& addParent(UUID parentID);

        void removeParent();

        template<typename T>
        T* tryGetComponent() {
            return m_Registry->try_get<T>(m_Handle);
        }

        template<typename ...Args>
        auto tryGetComponents() {
            return m_Registry->try_get<Args...>(m_Handle);
        }

        template<typename T>
        void removeComponent() {
            m_Registry->remove<T>(m_Handle);
        }

        bool isValid() {
            if(m_Registry)
                return m_Registry->valid(m_Handle);
            return false;
        }


        void* addRuntimeComponent(const ComponentInfo& compInfo);
        RuntimeComponent* getRuntimeComponent(const std::string& compName);
        bool hasRuntimeComponent(const std::string& compName);
        void removeRuntimeComponent(const std::string& compName);

        template<typename T>
        T& getRuntimeField(const char* comp, const char* field) {
            auto* rc = getRuntimeComponent(comp);
            ENTT_ASSERT(rc, "Runtime component not found");
            return rc->getField<T>(field);
        }

        bool operator==(const Entity &other) const {
            return m_Handle == other.m_Handle;
        }
        explicit operator intptr_t() const {
            return static_cast<intptr_t>(m_Handle);
        }
        explicit operator entt::entity() const {
            return m_Handle;
        }

    private:
        Scene* m_Scene{};
        entt::registry* m_Registry{};
        entt::entity m_Handle{entt::null};
    };
}
namespace std {
    template<>
    struct hash<ZEN::Entity>{
        size_t operator()(const ZEN::Entity& entity) const {
            return hash<entt::entity>()((entt::entity)entity);
        }
    };
}