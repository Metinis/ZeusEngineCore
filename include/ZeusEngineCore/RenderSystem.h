#pragma once
#include <entt/entt.hpp>
#include "Renderer.h"

namespace ZEN {
	class Scene;
	class RenderSystem {
	public:
		explicit RenderSystem(Renderer *renderer, Scene *scene);
		void onUpdate();
		void onRender();
	private:
		void writeCameraData(glm::mat4& view, glm::mat4& projection);
		void bindSceneUBOs();
		void renderDrawables();
		void renderSkybox(const glm::mat4& view, const glm::mat4& projection);
		void updateWorldTransforms();
		Renderer* m_Renderer{};
		Scene* m_Scene{};
	};
}
