#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

Scene::Scene() {

}
entt::entity Scene::createEntity() {
	return m_Registry.create();
}