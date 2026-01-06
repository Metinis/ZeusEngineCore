#pragma once
#include "GLResourceManager.h"
#include "../IContext.h"

struct GLFWwindow;

namespace ZEN {
	struct FBO;

	class GLContext : public IContext {
	public:
		explicit GLContext();
		void drawMesh(IResourceManager& resourceManager, const GPUMesh& drawable) override;
		void clear(bool shouldClearColor, bool shouldClearDepth) override;
		void clearInt() override;
		void depthMask(bool val) override;
		void enableCullFace() override;
		void disableCullFace() override;
		void setViewport(uint32_t width, uint32_t height) override;
		void setViewport(uint32_t xCorner, uint32_t yCorner, uint32_t width, uint32_t height) override;
		void setDepthMode(eDepthModes depthMode) override;
		void swapBuffers() override;
		uint32_t readPixels(FBO fbo, float x, float y) override;
	private:
		GLFWwindow* m_WindowHandle{};
	};
}