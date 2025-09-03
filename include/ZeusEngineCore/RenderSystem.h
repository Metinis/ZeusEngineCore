#pragma once
#include <entt/entt.hpp>
#include "Renderer.h"

namespace ZEN {
	class RenderSystem {
	public:
		explicit RenderSystem(Renderer *renderer, const MaterialComp& shaderComp);
		void onUpdate(entt::registry& registry);
		void onRender(entt::registry& registry);
	private:
		Renderer* m_Renderer{};
		UniformComp m_ViewUBO{};
		UniformComp m_InstanceUBO{};
		MaterialComp m_DefaultShader{};
	};
}