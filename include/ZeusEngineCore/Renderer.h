#pragma once
#include "InputEvents.h"
#include "Layer.h"
#include "../src/Systems/Renderer/OpenGL/GLContext.h"
#include "ZeusEngineCore/Components.h"

struct GLFWwindow;
namespace ZEN {
	struct SceneViewResizeEvent;
	struct WindowResizeEvent;
	class EventDispatcher;

	struct GlobalUBO {
		//glm::vec3 lightDir; float _pad0;
		glm::vec3 lightPos; float _pad0;
		glm::vec3 cameraPos; float _pad1;
		//alignas(16) float time;
		glm::vec3 ambientColor; float _pad2;
	};

	struct MaterialUBO {
		glm::vec4 albedo{1.0f};        // RGB color, w unused
		glm::vec4 params{0.0f};        // x = metallic, y = roughness, z = ao, w = metalFlag
	};

	struct Texture {
		uint32_t textureID; //id inside resource manager
	};
	struct RBO {
		uint32_t rboID;
	};
	struct FBO {
		uint32_t fboID;
	};

	class Renderer : public Layer {
	public:
		explicit Renderer();

		template<typename T>
		void writeToUBO(uint32_t uboID, T ubo) {
			auto const bytes = std::bit_cast<std::array<std::byte, sizeof(ubo)>>(ubo);
			m_ResourceManager->writeToUBO(uboID, bytes);
		}
		void beginFrame();
		void bindDefaultFBO();

		void renderToCubeMapHDR(uint32_t cubemapTexID, uint32_t eqToCubeMapShader, uint32_t hdrTexID, const GPUMesh& drawable);
		void renderToIrradianceMap(uint32_t cubemapTexID, uint32_t irradianceTexID, uint32_t irradianceShader,const GPUMesh &drawable);
		void renderToPrefilterMap(uint32_t cubemapTexID, uint32_t prefilterTexID, uint32_t prefilterShader,const GPUMesh &drawable);
		void renderToBRDFLUT(uint32_t brdfTexID, uint32_t brdfShader, const GPUMesh& drawable);
		void renderToScreenQuad(uint32_t quadShader, const GPUMesh& drawable);

		void endFrame();
		void setSize(float width, float height);
		glm::vec2 getSize() {return {m_Width, m_Height};}
		float getAspectRatio() const {return m_AspectRatio;}
		void setAspectRatio(float aspectRatio) { m_AspectRatio = aspectRatio; }
		Texture& getColorTexture() {return m_ColorTex;}
		void* getColorTextureHandle();
		IResourceManager* getResourceManager() {return m_ResourceManager.get();}
	private:
		friend class RenderSystem;
		GLFWwindow* m_Window{};
		std::unique_ptr<IContext> m_Context{};
		std::unique_ptr<IResourceManager> m_ResourceManager{};
		UniformBuffer m_ViewUBO{};
		UniformBuffer m_InstanceUBO{};
		UniformBuffer m_GlobalUBO{};
		UniformBuffer m_MaterialUBO{};
		MaterialComp m_DefaultShader{};

		FBO m_MainFBO{};

		FBO m_CaptureFBO{};
		RBO m_CaptureRBO{};

		Texture m_ColorTex{};
		RBO m_DepthRBO{};

		bool m_Resized{};
		float m_AspectRatio{};
		float m_Width{};
		float m_Height{};
	};
}
