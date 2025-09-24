#pragma once
#include "GLResourceManager.h"
#include "../IContext.h"

struct GLFWwindow;

namespace ZEN {
	class GLContext : public IContext{
	public:
		explicit GLContext(GLFWwindow* window);
		void drawMesh(IResourceManager& resourceManager, const MeshDrawable& drawable) override;
		void clear(bool shouldClearColor, bool shouldClearDepth) override;
		void depthMask(bool val) override;
		void setDepthMode(eDepthModes depthMode) override;
		void swapBuffers() override;
	private:
		GLFWwindow* m_WindowHandle{};
	};
}