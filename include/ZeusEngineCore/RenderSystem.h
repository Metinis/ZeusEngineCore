#pragma once
#include "EventDispatcher.h"
#include "Renderer.h"

namespace ZEN {
	class Scene;
	class ModelLibrary;
	struct RemoveMeshEvent;
	struct RemoveMeshCompEvent;
	struct RemoveMeshDrawableEvent;
	class RenderSystem {
	public:
		explicit RenderSystem(Renderer *renderer, Scene *scene, ModelLibrary* library,
			EventDispatcher* dispatcher);
		void onUpdate();
		void onRender();
	private:
		void writeCameraData(glm::mat4& view, glm::mat4& projection);
		void bindSceneUBOs();
		void renderDrawables();
		void renderSkybox(const glm::mat4& view, const glm::mat4& projection);
		void updateWorldTransforms();
		void onMeshRemove(RemoveMeshEvent& e);
		void onMeshCompRemove(RemoveMeshCompEvent& e);
		void onMeshDrawableRemove(RemoveMeshDrawableEvent& e);
		Renderer* m_Renderer{};
		ModelLibrary* m_Library{};
		Scene* m_Scene{};
		EventDispatcher* m_Dispatcher{};
	};
}
