#pragma once
#include "Layer.h"
#include "Renderer.h"
#include "ZeusEngineCore/ModelLibrary.h"

namespace ZEN {
	class Scene;
	struct RemoveMeshEvent;
	struct RemoveMeshCompEvent;
	struct RemoveMeshDrawableEvent;
	struct ToggleDrawNormalsEvent;
	class RenderSystem : public Layer {
	public:
		explicit RenderSystem(Renderer *renderer, Scene *scene, ModelLibrary* library);
		void onUpdate(float deltaTime) override;
		void onRender() override;
		void onEvent(Event& event) override;
		void toggleDrawNormals() { m_DrawNormals = !m_DrawNormals; }
	private:
		bool onPlayModeEvent(RunPlayModeEvent& e);
		void writeCameraData(glm::mat4& view, glm::mat4& projection);
		void setLightData(glm::vec3 cameraPos);
		void bindSceneUBOs();

		void renderDrawables();
		void renderDrawablesToShader(uint32_t shaderID);
		void renderSkybox(const glm::mat4& view, const glm::mat4& projection);

		void updateWorldTransforms();

		uint32_t m_IrradianceMapID{};
		uint32_t m_PrefilterMapID{};
		uint32_t m_BRDFLUTID{};
		uint32_t m_QuadShaderID{};
		MeshDrawable m_CubeDrawable{};
		MeshDrawable m_QuadDrawable{};
		Renderer* m_Renderer{};
		ModelLibrary* m_Library{};
		Scene* m_Scene{};
		bool m_DrawNormals{};
		bool m_IsPlaying{};
	};
}
