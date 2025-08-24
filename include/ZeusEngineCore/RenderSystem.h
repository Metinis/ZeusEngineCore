#pragma once
#include <entt/entt.hpp>

namespace ZEN {
	class RenderSystem {
	public:
		RenderSystem() = default;
		void render(entt::registry& registry);
	private:

	};
}