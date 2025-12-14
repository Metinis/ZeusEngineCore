#pragma once
#include "Layer.h"
#include "Renderer.h"
#include "ZeusEngineCore/AssetLibrary.h"
#include "ZeusEngineCore/AssetHandle.h"

namespace ZEN {
	class Scene;
	struct RemoveMeshEvent;
	struct RemoveMeshCompEvent;
	struct RemoveMeshDrawableEvent;
	struct ToggleDrawNormalsEvent;
	class RenderSystem : public Layer {
	public:
		explicit RenderSystem(Renderer *renderer, Scene *scene);
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

		AssetHandle<TextureData> m_IrradianceMapID{};
		AssetHandle<TextureData> m_PrefilterMapID{};
		AssetHandle<TextureData> m_BRDFLUTID{};
		AssetHandle<ShaderData> m_QuadShaderID{};
		AssetHandle<ShaderData> m_NormalsShaderID{};
		AssetHandle<MeshDrawable> m_CubeDrawable{};
		AssetHandle<MeshDrawable> m_QuadDrawable{};
		IResourceManager* m_ResourceManager{};
		Renderer* m_Renderer{};
		Scene* m_Scene{};
		bool m_DrawNormals{};
		bool m_IsPlaying{};
	};
}
