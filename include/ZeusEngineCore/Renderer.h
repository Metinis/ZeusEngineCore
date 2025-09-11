#pragma once
#include "../src/Systems/Renderer/OpenGL/GLContext.h"

namespace ZEN {
	struct GlobalUBO {
		glm::vec3 lightDir; float _pad0;
		glm::vec3 cameraPos; float _pad1;
		//alignas(16) float time;
		glm::vec3 ambientColor; float _pad2;
	};

	struct MaterialUBO {
		glm::vec2 specularAndShininess{0.5f, 32}; float _pad[2];
	};

	class Renderer {
	public:
		explicit Renderer(eRendererAPI api, GLFWwindow* window);
		void beginFrame();
		void endFrame();
		void setDefaultShader(const MaterialComp& shader);
		IContext* getContext() {return m_Context.get();}
		IResourceManager* getResourceManager() {return m_ResourceManager.get();}
		UniformComp& getViewUBO() {return m_ViewUBO;}
		UniformComp& getInstanceUBO() {return m_InstanceUBO;}
		UniformComp& getGlobalUBO() {return m_GlobalUBO;}
		UniformComp& getMaterialUBO() {return m_MaterialUBO;}
		MaterialComp& getDefaultShader() {return m_DefaultShader;}
	private:
		std::unique_ptr<IContext> m_Context{};
		std::unique_ptr<IResourceManager> m_ResourceManager{};
		UniformComp m_ViewUBO{};
		UniformComp m_InstanceUBO{};
		UniformComp m_GlobalUBO{};
		UniformComp m_MaterialUBO{};
		MaterialComp m_DefaultShader{};
	};
}