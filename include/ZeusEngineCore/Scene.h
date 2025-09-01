#pragma once
#include <entt/entt.hpp>
#include "ZeusEngineCore/RenderSystem.h"

namespace ZEN {

	class Scene {

	public:
		Scene();
		entt::entity createEntity();
		entt::registry& getRegistry();
	private:
		entt::registry m_Registry{};
	};
}