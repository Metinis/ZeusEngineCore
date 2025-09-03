#pragma once
#include <entt/entt.hpp>
#include "Renderer.h"

namespace ZEN {
	class Scene;
	class RenderSystem {
	public:
		explicit RenderSystem(Renderer *renderer, Scene *scene);
		void onUpdate(entt::registry& registry);
		void onRender(entt::registry& registry);
	private:
		Renderer* m_Renderer{};
		Scene* m_Scene{};
	};
}
