#pragma once
#include <entt/entt.hpp>
#include "ZeusEngineCore/RenderSystem.h"

namespace ZEN {

	class Scene {

	public:
		Scene();
		void createDefaultScene(const std::string& resourceRoot, Renderer* renderer);
		entt::entity createEntity();
		entt::registry& getRegistry();
		entt::dispatcher& getDispatcher();
		glm::vec3 getLightPos() {return lightPos;}
		glm::vec3 getLightDir() {return lightDir;}
		glm::vec3 getAmbientColor() {return ambientColor;}
	private:
		entt::registry m_Registry{};
		entt::dispatcher m_Dispather{};
		glm::vec3 lightPos{1.0f, 5.0f, 1.0f};
		glm::vec3 lightDir{-0.2f, -1.0f, 0.3f};
		glm::vec3 ambientColor{0.5f, 0.5f, 0.5f};
	};
}