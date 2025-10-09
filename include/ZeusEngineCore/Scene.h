#pragma once
#include <entt/entt.hpp>
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/Entity.h"


namespace ZEN {
	class ModelLibrary;
	class ModelImporter;
	class ZEngine;
	struct RemoveMeshEvent;
	struct RemoveMaterialEvent;
	struct RemoveTextureEvent;

	class Scene {

	public:
		Scene(EventDispatcher& dispatcher);
		void createDefaultScene(const std::string& resourceRoot, ZEngine* engine);
		Entity createEntity(const std::string& name = "");

		template<typename ...Args>
		auto getEntities() {
			auto view = m_Registry.view<Args...>();

			return view | std::views::transform(
				[this](entt::entity entity) { return makeEntity(entity); }
			);
		}
		glm::vec3 getLightPos() {return lightPos;}
		glm::vec3 getLightDir() {return lightDir;}
		glm::vec3 getAmbientColor() {return ambientColor;}
	private:
		entt::registry m_Registry{};
		glm::vec3 lightPos{1.0f, 5.0f, 1.0f};
		glm::vec3 lightDir{-0.2f, -1.0f, 0.3f};
		glm::vec3 ambientColor{0.5f, 0.5f, 0.5f};

		Entity makeEntity(entt::entity entity);

		void onRemoveMesh(RemoveMeshEvent& e);
		void onRemoveMaterial(RemoveMaterialEvent& e);
		void onRemoveTexture(RemoveTextureEvent& e);

		friend class Entity;
	};
}
