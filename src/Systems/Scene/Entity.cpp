#include "ZeusEngineCore/Entity.h"
#include "ZeusEngineCore/Scene.h"

ZEN::Entity::Entity(Scene *scene, entt::entity handle) : m_Registry(&scene->m_Registry), m_Handle(handle){
    assert(m_Registry && "Entity constructed with null registry!");
}

ZEN::Entity::Entity(entt::registry *registry, entt::entity handle) : m_Registry(registry), m_Handle(handle) {
    assert(m_Registry && "Entity constructed with null registry!");

}
