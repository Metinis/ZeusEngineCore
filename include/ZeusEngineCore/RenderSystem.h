#pragma once
#include <entt/entt.hpp>
#include "Renderer.h"

namespace ZEN {
	class RenderSystem {
	public:
		explicit RenderSystem(Renderer *renderer, const ShaderComp& shaderComp);
		void onUpdate(entt::registry& registry);
		void onRender(entt::registry& registry);
	private:
		Renderer* m_Renderer{};
		UniformComp m_ViewUBO{};
		ShaderComp m_DefaultShader{};
	};
}