#pragma once
#include "ZeusEngineCore/MeshComp.h"

struct GLFWwindow;

namespace ZEN {
	class GLContext {
	public:
		GLContext(GLFWwindow* window);
		void DrawMesh(const MeshRenderable& meshRenderable);
		void SwapBuffers();
	private:
		GLFWwindow* m_WindowHandle{};
	};
}