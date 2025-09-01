#pragma once
#include "../src/Systems/Renderer/OpenGL/GLContext.h"

namespace ZEN {
	class Renderer {
	public:
		explicit Renderer(eRendererAPI api, GLFWwindow* window);
		void beginFrame();
		void endFrame();
		IContext* getContext() {return m_Context.get();}
	private:
		std::unique_ptr<IContext> m_Context;
	};
}