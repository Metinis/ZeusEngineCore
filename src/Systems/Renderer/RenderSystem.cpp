#include "ZeusEngineCore/RenderSystem.h"

using namespace ZEN;

RenderSystem::RenderSystem(Renderer *renderer, const ShaderComp& shaderComp) :
m_Renderer(renderer), m_DefaultShader(shaderComp) {
    m_ViewUBO.uboID = m_Renderer->getContext().getResourceManager().createUBO(0);
}
void RenderSystem::onUpdate(entt::registry &registry) {
    //write camera data
    auto view = registry.view<CameraComp>;
    for (auto entity: view) {
        auto& camera = registry.get<CameraComp>(entity);
        if(camera.isPrimary) {
            //update view ubo
            glm::mat4 vpMat = camera.projection * camera.view;
            auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
            m_Renderer->getContext().getResourceManager().writeToUBO(m_ViewUBO.uboID, bytes);
        }
    }

    //create buffers for all meshes without drawable comps
    view = registry.view<MeshComp>(entt::exclude<MeshDrawableComp>);
    for (auto entity : view) {
        uint32_t shaderID = 0;
        if (registry.all_of<ShaderComp>(entity)) {
            shaderID = registry.get<ShaderComp>(entity).shaderID;
        } else {
            // Fallback shader if none exists
            shaderID = m_DefaultShader.shaderID;
            registry.emplace<ShaderComp>(entity, ShaderComp{ shaderID });
        }

        auto &mesh = view.get<MeshComp>(entity);
        MeshDrawableComp meshDrawable {
            .indexCount = mesh.indices.size(),
            .meshID = m_Renderer->getContext().
                        getResourceManager().createMeshDrawable(mesh),
        };
        registry.emplace<MeshDrawableComp>(entity, meshDrawable);
    }
}

void RenderSystem::onRender(entt::registry& registry) {
    //bind uniform vp matrix
    m_Renderer->getContext().getResourceManager().bindUBO(m_ViewUBO.uboID);

    //render all meshes with drawable comps
    auto view = registry.view<MeshDrawableComp, ShaderComp>();
    uint32_t lastShaderID{};
    for(auto& entity : view) {
        if(view.get<ShaderComp>(entity).shaderID != lastShaderID) {
            lastShaderID = view.get<ShaderComp>(entity).shaderID;
            m_Renderer->getContext().getResourceManager().bindShader(lastShaderID);
        }
        m_Renderer->getContext().drawMesh(view.get<MeshDrawableComp>(entity));
    }
}
