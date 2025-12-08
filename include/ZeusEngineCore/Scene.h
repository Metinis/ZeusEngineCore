#pragma once
#include <entt/entt.hpp>

#include "InputEvents.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/Entity.h"


namespace ZEN {
	class ModelLibrary;
	class ModelImporter;
	class ZEngine;
	struct RemoveMeshEvent;
	struct RemoveMaterialEvent;
	struct RemoveTextureEvent;

	class Scene : public Layer{

	public:
		Scene();
		void createDefaultScene(ZEngine* engine);
		Entity createEntity(const std::string& name = "");
		void removeEntity(Entity entity);

		template<typename ...Args>
		auto getEntities() {
			auto view = m_Registry.view<Args...>();

			return view | std::views::transform(
				[this](entt::entity entity) { return makeEntity(entity); }
			);
		}
		void onEvent(Event& event) override;

		glm::vec3 getLightPos() {return m_LightPos;}
		glm::vec3 getLightDir() {return m_LightDir;}
		glm::vec3 getAmbientColor() {return m_AmbientColor;}
	private:
		entt::registry m_Registry{};
		ModelLibrary* m_ModelLibrary{};
		glm::vec3 m_LightPos{1.0f, 5.0f, 1.0f};
		glm::vec3 m_LightDir{-0.2f, -1.0f, 0.3f};
		glm::vec3 m_AmbientColor{0.01f, 0.01f, 0.01f};

		Entity makeEntity(entt::entity entity);

		template<typename T>
		void removeResource(const std::string& resourceName) {
			auto view = getEntities<T>();
            for (auto entity : view) {
                if(entity.template getComponent<T>().name != resourceName) {
                    continue;
                }
                entity.template removeComponent<T>();
            }
		}

		bool onRemoveResource(RemoveResourceEvent& e);
		void onMeshCompRemove(entt::registry& registry, entt::entity entity);
		void onMeshDrawableRemove(entt::registry& registry, entt::entity entity);

		friend class Entity;
		friend class SceneSerializer;
	};
}
