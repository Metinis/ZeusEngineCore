#pragma once
#include "GLResourceManager.h"
#include "../IContext.h"

struct GLFWwindow;

namespace ZEN {
	class GLContext : public IContext{
	public:
		explicit GLContext(GLFWwindow* window);
		IResourceManager& getResourceManager() override {return m_ResourceManager;}
		void drawMesh(const MeshDrawableComp& meshRenderable) override;
		void clear(bool shouldClearColor, bool shouldClearDepth) override;
		void depthMask(bool val) override;
		void swapBuffers() override;
	private:
		GLResourceManager m_ResourceManager{};
		GLFWwindow* m_WindowHandle{};
	};
}