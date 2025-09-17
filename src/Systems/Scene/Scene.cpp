#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

Scene::Scene() {

}
entt::entity Scene::createEntity() {
	return m_Registry.create();
}

entt::registry& Scene::getRegistry() {
	return m_Registry;
}

entt::dispatcher & Scene::getDispatcher() {
	return m_Dispather;
}
