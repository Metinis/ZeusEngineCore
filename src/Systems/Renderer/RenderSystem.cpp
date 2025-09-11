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
    glm::mat4 view;
    glm::mat4 projection;
    auto cameraView = registry.view<CameraComp, TransformComp>();
    for (auto entity: cameraView) {
        auto& camera = registry.get<CameraComp>(entity);
        auto& transform = registry.get<TransformComp>(entity);
        if(camera.isPrimary) {
            //update view ubo
            view = transform.getViewMatrix();
            projection = camera.projection;
            //glm::mat4 vpMat = projection * view;
            //auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
            //m_Renderer->getContext()->getResourceManager().
            //    writeToUBO(m_Renderer->getViewUBO().uboID, bytes);
            //update global ubo, todo maybe use a light component
            GlobalUBO globalUBO {
                .lightDir = m_Scene->getLightDir(),
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

    //draw skybox
    auto skyboxView = registry.view<SkyboxComp, MeshDrawableComp>();
    for (auto entity: skyboxView) {
        m_Renderer->getContext()->depthMask(false);
        glm::mat4 viewCube = glm::mat4(glm::mat3(view)); // remove translation
        glm::mat4 vp = projection * viewCube;
        auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vp)>>(vp);
        m_Renderer->getContext()->getResourceManager().
            writeToUBO(m_Renderer->getViewUBO().uboID, bytes);
        //bind shader
        auto& skyboxComp = skyboxView.get<SkyboxComp>(entity);
        m_Renderer->getContext()->getResourceManager().bindShader(skyboxComp.shaderID);

        //bind cubemap texture
        m_Renderer->getContext()->getResourceManager().bindCubeMapTexture(skyboxComp.textureID);

        m_Renderer->getContext()->drawMesh(skyboxView.get<MeshDrawableComp>(entity));
        m_Renderer->getContext()->depthMask(true);
    }
    glm::mat4 vpMat = projection * view;
    auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
    m_Renderer->getContext()->getResourceManager().
        writeToUBO(m_Renderer->getViewUBO().uboID, bytes);

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
    auto viewDraw = registry.view<MeshDrawableComp, MaterialComp, TransformComp>();
    uint32_t lastMaterialID{};
    for(auto& entity : viewDraw) {
        if(viewDraw.get<MaterialComp>(entity).shaderID != lastMaterialID) {
            //bind shader
            auto& material = viewDraw.get<MaterialComp>(entity);
            lastMaterialID = material.shaderID;
            m_Renderer->getContext()->getResourceManager().bindShader(lastMaterialID);

            //write to material ubo
            MaterialUBO materialUBO {
                .specularAndShininess = {0.5, 32}
            };
            auto const materialBytes = std::bit_cast<std::array<std::byte,
                sizeof(materialUBO)>>(materialUBO);
            m_Renderer->getContext()->getResourceManager().
                writeToUBO(m_Renderer->getMaterialUBO().uboID, materialBytes);

            //bind material texture
            m_Renderer->getContext()->getResourceManager().bindTexture(material.textureID, 0);
            m_Renderer->getContext()->getResourceManager().bindTexture(material.specularTexID, 1);
        }
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = viewDraw.get<TransformComp>(entity);
        auto const bytes = std::bit_cast<std::array<std::byte,
            sizeof(transform.getModelMatrix())>>(transform.getModelMatrix());
        m_Renderer->getContext()->getResourceManager().writeToUBO(m_Renderer->getInstanceUBO().uboID, bytes);

        //todo submit mesh to renderer
        m_Renderer->getContext()->drawMesh(viewDraw.get<MeshDrawableComp>(entity));
    }
}
