#pragma once
#include <entt/entt.hpp>
#include "ZeusEngineCore/core/InputEvents.h"
#include "ZeusEngineCore/scripting/CompRegistry.h"
#include "ZeusEngineCore/scripting/SystemManager.h"
#include "ZeusEngineCore/engine/RenderSystem.h"
#include "ZeusEngineCore/engine/Entity.h"

namespace ZEN {
	class AssetLibrary;
	class ModelImporter;
	class ZEngine;
	struct RemoveMeshEvent;
	struct RemoveMaterialEvent;
	struct RemoveTextureEvent;

	class ZEN_API Scene : public Layer {

	public:
		Scene();
		~Scene();
		void onUpdate(float dt) override;
		void createDefaultScene();
		Entity createEntity(const std::string& name = "");
		Entity createEntity(const std::string& name, UUID id);
		Entity getEntity(UUID id);
		Entity getEntityByRegistryID(uint32_t registryID);
		Entity getSceneCamera();
		bool isDescendantOf(Entity parent, Entity possibleChild);
		void removeEntity(Entity entity);

		template<typename ...Args>
		auto getEntities() {
			auto view = m_Registry.view<Args...>();

			return view | std::views::transform(
				[this](entt::entity entity) { return makeEntity(entity); }
			);
		}
		std::vector<Entity> getEntities(std::string_view name) const;

		bool onPlayMode(RunPlayModeEvent& e);
		void onEvent(Event& event) override;

		//todo move this out of scene, probably in project or as static singletons

	private:
		entt::registry m_Registry{};
		std::unordered_map<Entity, std::unordered_map<std::string, RuntimeComponent>> m_RuntimeComponents;
		AssetLibrary* m_ModelLibrary{};
		PhysicsSystem* m_PhysicsSystem{};

		bool m_PlayMode{false};
		bool m_LoadedScene{false};

		Entity makeEntity(entt::entity entity);
		int computeDepth(Entity e);
		void updateWorldTransforms();

		friend class Entity;
		friend class SceneSerializer;
	};
}
