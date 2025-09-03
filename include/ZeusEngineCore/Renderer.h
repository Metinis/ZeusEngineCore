#pragma once
#include "../src/Systems/Renderer/OpenGL/GLContext.h"

namespace ZEN {
	struct GlobalUBO {
		glm::vec3 lightPos;
		float _pad1;            //need to pad to 16bytes for std140, 1 float = 4 bytes
		glm::vec3 cameraPos;
		float _pad2;
		float time;
		float _pad3[3];
		glm::vec3 ambientColor;
		float _pad4;
	};


	class Renderer {
	public:
		explicit Renderer(eRendererAPI api, GLFWwindow* window);
		void beginFrame();
		void endFrame();
		void setDefaultShader(const MaterialComp& shader);
		IContext* getContext() {return m_Context.get();}
		UniformComp& getViewUBO() {return m_ViewUBO;}
		UniformComp& getInstanceUBO() {return m_InstanceUBO;}
		UniformComp& getGlobalUBO() {return m_GlobalUBO;}
		MaterialComp& getDefaultShader() {return m_DefaultShader;}
	private:
		std::unique_ptr<IContext> m_Context;
		UniformComp m_ViewUBO{};
		UniformComp m_InstanceUBO{};
		UniformComp m_GlobalUBO{};
		MaterialComp m_DefaultShader{};
	};
}