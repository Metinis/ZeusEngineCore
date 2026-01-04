#include "ZeusEngineCore/Entity.h"
#include "ZeusEngineCore/Scene.h"

using namespace ZEN;
Entity::Entity(Scene *scene, entt::entity handle) : m_Scene(scene), m_Registry(&scene->m_Registry), m_Handle(handle){
    assert(m_Registry && "Entity constructed with null registry!");
}

void* Entity::addRuntimeComponent(const ComponentInfo &compInfo) {
    auto& entityMap = m_Scene->m_RuntimeComponents[*this];
    auto& storage = entityMap[compInfo.name];
    storage.buffer.resize(compInfo.size);
    storage.info = &compInfo;
    std::memset(storage.buffer.data(), 0, compInfo.size);
    return storage.buffer.data();
}

RuntimeComponent * Entity::getRuntimeComponent(const std::string &compName) {
    auto entityIt = m_Scene->m_RuntimeComponents.find(*this);
    if (entityIt == m_Scene->m_RuntimeComponents.end()) return nullptr;
    auto compIt = entityIt->second.find(compName);
    if (compIt == entityIt->second.end()) return nullptr;
    return &compIt->second;
}

bool Entity::hasRuntimeComponent(const std::string &compName) {
    auto entityIt = m_Scene->m_RuntimeComponents.find(*this);
    if (entityIt == m_Scene->m_RuntimeComponents.end()) return false;
    auto compIt = entityIt->second.find(compName);
    if (compIt == entityIt->second.end()) return false;
    return true;
}

void Entity::removeRuntimeComponent(const std::string &compName) {
    auto& entityMap = m_Scene->m_RuntimeComponents[*this];
    entityMap.erase(compName);
}

ParentComp& Entity::addParent(const ParentComp &pc) {
    return addParent(pc.parentID);
}

ParentComp& Entity::addParent(UUID parentID) {
    if (hasComponent<TransformComp>()) {
        auto& tc = getComponent<TransformComp>();
        auto parentE = m_Scene->getEntity(parentID);

        if (parentE.isValid() && parentE.hasComponent<TransformComp>()) {
            auto& ptc = parentE.getComponent<TransformComp>();
            tc.localPosition =
                glm::inverse(ptc.worldMatrix) *
                glm::vec4(tc.localPosition, 1.0f);
        }
    }
    return m_Registry->emplace<ParentComp>(m_Handle, ParentComp{parentID});
}

void Entity::removeParent() {
    if (hasComponent<TransformComp>() && hasComponent<ParentComp>()) {
        auto& tc = getComponent<TransformComp>();
        auto& pc = getComponent<ParentComp>();
        auto parentE = m_Scene->getEntity(pc.parentID);

        if (parentE.isValid() && parentE.hasComponent<TransformComp>()) {
            auto& ptc = parentE.getComponent<TransformComp>();
            tc.localPosition =
                ptc.worldMatrix *
                glm::vec4(tc.localPosition, 1.0f);
        }
    }
    m_Registry->remove<ParentComp>(m_Handle);
}


