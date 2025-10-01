#include "ZeusEngineCore/Entity.h"
#include "ZeusEngineCore/Scene.h"

ZEN::Entity::Entity(Scene *scene, entt::entity handle) : m_Registry(&scene->m_Registry), m_Handle(handle){

}
