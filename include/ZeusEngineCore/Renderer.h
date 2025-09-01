#pragma once
#include "../src/Systems/Renderer/OpenGL/GLContext.h"

namespace ZEN {
	class Renderer {
	public:
		explicit Renderer(GLFWwindow* window);
		void beginFrame();
		void endFrame();
		GLContext& getContext() {return m_Context;}
	private:
		GLContext m_Context;
	};
}