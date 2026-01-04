#include "ZeusEngineCore/Entity.h"
#include "ZeusEngineCore/Scene.h"

ZEN::Entity::Entity(Scene *scene, entt::entity handle) : m_Scene(scene), m_Registry(&scene->m_Registry), m_Handle(handle){
    assert(m_Registry && "Entity constructed with null registry!");
}

void* ZEN::Entity::addRuntimeComponent(const ComponentInfo &compInfo) {
    auto& entityMap = m_Scene->m_RuntimeComponents[*this];
    auto& storage = entityMap[compInfo.name];
    storage.buffer.resize(compInfo.size);
    storage.info = &compInfo;
    std::memset(storage.buffer.data(), 0, compInfo.size);
    return storage.buffer.data();
}

ZEN::RuntimeComponent * ZEN::Entity::getRuntimeComponent(const std::string &compName) {
    auto entityIt = m_Scene->m_RuntimeComponents.find(*this);
    if (entityIt == m_Scene->m_RuntimeComponents.end()) return nullptr;
    auto compIt = entityIt->second.find(compName);
    if (compIt == entityIt->second.end()) return nullptr;
    return &compIt->second;
}

void ZEN::Entity::removeRuntimeComponent(const std::string &compName) {
    auto& entityMap = m_Scene->m_RuntimeComponents[*this];
    entityMap.erase(compName);
}

/*ZEN::Entity::Entity(entt::registry *registry, entt::entity handle) : m_Registry(registry), m_Handle(handle) {
    assert(m_Registry && "Entity constructed with null registry!");

}*/
