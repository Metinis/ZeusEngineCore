#pragma once
#include "ZeusEngineCore/asset/GPUHandle.h"
#include "ZeusEngineCore/core/Layer.h"
#include "Renderer.h"

namespace ZEN {
	class Scene;
	struct RemoveMeshEvent;
	struct RemoveMeshCompEvent;
	struct RemoveMeshDrawableEvent;
	struct ToggleDrawNormalsEvent;
	class RenderSystem : public Layer {
	public:
		explicit RenderSystem();
		void onUpdate(float deltaTime) override;
		void onRender() override;
		void onEvent(Event& event) override;
		void toggleDrawNormals() { m_DrawNormals = !m_DrawNormals; }
	private:
		void initSkyboxAssets(SkyboxComp& comp);
		bool onPlayModeEvent(RunPlayModeEvent& e);
		void writeCameraData(glm::mat4& view, glm::mat4& projection);
		void setLightData(glm::vec3 cameraPos);
		void bindSceneUBOs();

		void renderDrawables();
		void renderDrawablesToShader(uint32_t shaderID);
		void renderSkybox(const glm::mat4& view, const glm::mat4& projection);

		void updateWorldTransforms();

		GPUHandle<GPUTexture> m_IrradianceMapID;
		GPUHandle<GPUTexture> m_PrefilterMapID;
		GPUHandle<GPUTexture> m_BRDFLUTID;
		GPUHandle<GPUShader> m_QuadShaderID;
		GPUHandle<GPUShader> m_NormalsShaderID;
		GPUHandle<GPUMesh> m_CubeDrawable;
		GPUHandle<GPUMesh> m_QuadDrawable;
		IResourceManager* m_ResourceManager{};
		Renderer* m_Renderer{};
		Scene* m_Scene{};
		bool m_DrawNormals{};
		bool m_IsPlaying{};
	};
}
