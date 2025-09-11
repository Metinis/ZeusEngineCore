#pragma once
#include <entt/entt.hpp>
#include "ZeusEngineCore/RenderSystem.h"

namespace ZEN {

	class Scene {

	public:
		Scene();
		entt::entity createEntity();
		entt::registry& getRegistry();
		glm::vec3 getLightPos() {return lightPos;}
		glm::vec3 getLightDir() {return lightDir;}
		glm::vec3 getAmbientColor() {return ambientColor;}
	private:
		entt::registry m_Registry{};
		glm::vec3 lightPos{1.0f, 5.0f, 1.0f};
		glm::vec3 lightDir{-0.2f, -1.0f, -0.3f};
		glm::vec3 ambientColor{0.25f, 0.25f, 0.25f};
	};
}