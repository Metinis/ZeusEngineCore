#pragma once
#include "MeshComp.h"
namespace ZEN {
	class Renderer {
	public:
		Renderer() = default;
		void drawMesh(const MeshDrawableComp& meshDrawable);
	private:

	};
}