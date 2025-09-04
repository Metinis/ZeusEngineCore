#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

RenderSystem::RenderSystem(Renderer *renderer, Scene *scene) :
m_Renderer(renderer), m_Scene(scene){

}
void RenderSystem::onUpdate(entt::registry &registry) {
    //create buffers for all meshes without drawable comps
    auto meshView = registry.view<MeshComp>(entt::exclude<MeshDrawableComp>);
    for (auto entity : meshView) {
        uint32_t shaderID = 0;
        if (registry.all_of<MaterialComp>(entity)) {
            shaderID = registry.get<MaterialComp>(entity).shaderID;
        } else {
            // Fallback shader if none exists
            shaderID = m_Renderer->getDefaultShader().shaderID;
            registry.emplace<MaterialComp>(entity, MaterialComp{ shaderID });
        }

        auto &mesh = meshView.get<MeshComp>(entity);
        MeshDrawableComp meshDrawable {
            .indexCount = mesh.indices.size(),
            .meshID = m_Renderer->getContext()->
                        getResourceManager().createMeshDrawable(mesh),
        };
        registry.emplace<MeshDrawableComp>(entity, meshDrawable);
    }
}

void RenderSystem::onRender(entt::registry& registry) {
    //write camera data
    auto cameraView = registry.view<CameraComp, TransformComp>();
    for (auto entity: cameraView) {
        auto& camera = registry.get<CameraComp>(entity);
        auto& transform = registry.get<TransformComp>(entity);
        if(camera.isPrimary) {
            //update view ubo
            glm::mat4 vpMat = camera.projection * transform.getViewMatrix();
            auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
            m_Renderer->getContext()->getResourceManager().
                writeToUBO(m_Renderer->getViewUBO().uboID, bytes);
            //update global ubo
            GlobalUBO globalUBO{
                .lightPos = m_Scene->getLightPos(),
                .cameraPos = transform.position,
                //.time = glfwGetTime(),
                .ambientColor = m_Scene->getAmbientColor(),
            };
            auto const globalBytes = std::bit_cast<std::array<std::byte,
                sizeof(globalUBO)>>(globalUBO);
            m_Renderer->getContext()->getResourceManager().
                writeToUBO(m_Renderer->getGlobalUBO().uboID, globalBytes);
        }
    }

    //bind uniform vp matrix
    m_Renderer->getContext()->getResourceManager().bindUBO(m_Renderer->getViewUBO().uboID);
    //bind instance ubo (different binding so its ok)
    m_Renderer->getContext()->getResourceManager().bindUBO(m_Renderer->getInstanceUBO().uboID);
    //bind global ubo (different binding so its ok)
    m_Renderer->getContext()->getResourceManager().bindUBO(m_Renderer->getGlobalUBO().uboID);
    //bind material ubo (different binding so its ok)
    m_Renderer->getContext()->getResourceManager().bindUBO(m_Renderer->getMaterialUBO().uboID);

    //render all meshes with drawable comps
    //todo sort by material
    auto view = registry.view<MeshDrawableComp, MaterialComp, TransformComp>();
    uint32_t lastMaterialID{};
    for(auto& entity : view) {
        if(view.get<MaterialComp>(entity).shaderID != lastMaterialID) {
            //bind shader
            auto& material = view.get<MaterialComp>(entity);
            lastMaterialID = material.shaderID;
            m_Renderer->getContext()->getResourceManager().bindShader(lastMaterialID);

            //write to material ubo
            MaterialUBO materialUBO{
                .specularStrength = material.specular,
            };
            auto const materialBytes = std::bit_cast<std::array<std::byte,
                sizeof(materialUBO)>>(materialUBO);
            m_Renderer->getContext()->getResourceManager().
                writeToUBO(m_Renderer->getMaterialUBO().uboID, materialBytes);

            //bind material texture
            m_Renderer->getContext()->getResourceManager().bindTexture(material.textureID);
        }
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = view.get<TransformComp>(entity);
        auto const bytes = std::bit_cast<std::array<std::byte,
            sizeof(transform.getModelMatrix())>>(transform.getModelMatrix());
        m_Renderer->getContext()->getResourceManager().writeToUBO(m_Renderer->getInstanceUBO().uboID, bytes);

        m_Renderer->getContext()->drawMesh(view.get<MeshDrawableComp>(entity));
    }
}
