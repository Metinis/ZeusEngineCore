#pragma once
#include "GLResourceManager.h"

struct GLFWwindow;

namespace ZEN {
	class GLContext {
	public:
		explicit GLContext(GLFWwindow* window);
		IResourceManager& getResourceManager() {return m_ResourceManager;}
		void drawMesh(const MeshDrawableComp& meshRenderable);
		void clear(bool shouldClearColor, bool shouldClearDepth);
		void swapBuffers();
	private:
		GLResourceManager m_ResourceManager{};
		GLFWwindow* m_WindowHandle{};
	};
}