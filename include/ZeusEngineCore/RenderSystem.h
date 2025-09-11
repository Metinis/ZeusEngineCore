#pragma once
#include <entt/entt.hpp>
#include "Renderer.h"

namespace ZEN {
	class Scene;
	class RenderSystem {
	public:
		explicit RenderSystem(Renderer *renderer, Scene *scene);
		void onUpdate(entt::registry& registry);
		void onRender(const entt::registry& registry);
	private:
		void writeCameraData(const entt::registry& registry, glm::mat4& view,
			glm::mat4& projection);
		void bindSceneUBOs();
		void renderDrawables(const entt::registry& registry);
		void renderSkybox(const entt::registry &registry, const glm::mat4& view,
			const glm::mat4& projection);
		Renderer* m_Renderer{};
		Scene* m_Scene{};
	};
}
