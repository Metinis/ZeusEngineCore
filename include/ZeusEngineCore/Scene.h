#pragma once
#include <entt/entt.hpp>
#include "InputEvents.h"
#include "scripting/CompRegistry.h"
#include "scripting/SystemManager.h"
#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/Entity.h"

namespace ZEN {
	class AssetLibrary;
	class ModelImporter;
	class ZEngine;
	struct RemoveMeshEvent;
	struct RemoveMaterialEvent;
	struct RemoveTextureEvent;

	class Scene : public Layer {

	public:
		Scene();
		void onUpdate(float dt) override;
		void createDefaultScene();
		Entity createEntity(const std::string& name = "");
		Entity createEntity(const std::string& name, UUID id);
		Entity getEntity(UUID id);
		void removeEntity(Entity entity);

		template<typename ...Args>
		auto getEntities() {
			auto view = m_Registry.view<Args...>();

			return view | std::views::transform(
				[this](entt::entity entity) { return makeEntity(entity); }
			);
		}
		std::vector<Entity> getEntities(const std::string& name);
		void* addRuntimeComponent(entt::entity entity, const ComponentInfo& compInfo);
		RuntimeComponent* getRuntimeComponent(entt::entity entity, const std::string& compName);

		RuntimeComponent* getRuntimeComponent(Entity entity, const std::string& compName) {
			return getRuntimeComponent(static_cast<entt::entity>(entity), compName);
		}

		template<typename T>
		T& getRuntimeField(Entity entity, const char* comp, const char* field) {
			auto* rc = getRuntimeComponent(entity, comp);
			ENTT_ASSERT(rc, "Runtime component not found");
			return getField<T>(*rc, field);
		}

		bool onPlayMode(RunPlayModeEvent& e);
		void onEvent(Event& event) override;
		//todo move this out of scene, probably in project or as static singletons
		SystemManager& getSystemManager() {return m_SystemManager;}
	private:
		entt::registry m_Registry{};
		std::unordered_map<entt::entity, std::unordered_map<std::string, RuntimeComponent>> m_RuntimeComponents;
		AssetLibrary* m_ModelLibrary{};
		SystemManager m_SystemManager{};
		bool m_PlayMode{false};

		Entity makeEntity(entt::entity entity);
		bool onRemoveResource(RemoveResourceEvent& e);
		void onMeshCompRemove(entt::registry& registry, entt::entity entity);
		void onMeshDrawableRemove(entt::registry& registry, entt::entity entity);

		friend class Entity;
		friend class SceneSerializer;
	};
}
