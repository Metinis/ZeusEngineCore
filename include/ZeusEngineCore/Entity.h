#pragma once
#include <entt/entt.hpp>

namespace ZEN {
    class Scene;
    class Entity {
    public:
        explicit Entity(Scene* scene, entt::entity handle);
        explicit Entity(entt::registry* registry, entt::entity handle);
        explicit Entity() = default;

        template<typename T>
        T& getComponent() {
            return m_Registry->get<T>(m_Handle);
        }

        template<typename T>
        bool hasComponent() {
            return m_Registry->any_of<T>(m_Handle);
        }

        template<typename T, typename ...Args>
        T& addComponent(Args... args) {
            return m_Registry->emplace<T>(m_Handle,
                std::forward<Args>(args)...);
        }

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
        entt::registry* m_Registry{};
        entt::entity m_Handle{entt::null};
    };
}