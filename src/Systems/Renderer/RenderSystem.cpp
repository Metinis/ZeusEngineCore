#include "ZeusEngineCore/RenderSystem.h"
#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

RenderSystem::RenderSystem(Renderer *renderer, Scene *scene) :
m_Renderer(renderer), m_Scene(scene){

}
void updateWorldTransforms(entt::registry& registry) {
    auto view = registry.view<TransformComp>();

    for (auto e : view) {
        auto &tc = view.get<TransformComp>(e);
        glm::mat4 local = tc.getLocalMatrix();


        if (auto parentComp = registry.try_get<ParentComp>(e)) {
            if(auto tranformComp = registry.try_get<TransformComp>(parentComp->parent)) {
                tc.worldMatrix = tranformComp->worldMatrix * local;
            }
        }
        else {
            tc.worldMatrix = local;
        }
    }
}

void RenderSystem::onUpdate(entt::registry &registry) {
    updateWorldTransforms(registry);
    //create buffers for all meshes without drawable comps
    auto meshView = registry.view<MeshComp>(entt::exclude<MeshDrawableComp>);
    for (auto entity : meshView) {
        uint32_t shaderID = 0;
        if (registry.all_of<MaterialComp>(entity)) {
            shaderID = registry.get<MaterialComp>(entity).shaderID;
        } else {
            // Fallback shader if none exists
            shaderID = m_Renderer->getDefaultShader().shaderID;
            registry.emplace<MaterialComp>(entity, MaterialComp{ shaderID, {0}, {0} });
        }

        auto &mesh = meshView.get<MeshComp>(entity);

        MeshDrawableComp drawableComp{};
        drawableComp.indexCount = mesh.indices.size();
        drawableComp.meshID = m_Renderer->getResourceManager()->createMeshDrawable(mesh);
        registry.emplace<MeshDrawableComp>(entity, drawableComp);
    }
}
void RenderSystem::writeCameraData(const entt::registry& registry, glm::mat4& view,
            glm::mat4& projection) {
    auto cameraView = registry.view<CameraComp, TransformComp>();
    for (auto entity: cameraView) {
        auto& camera = registry.get<CameraComp>(entity);
        auto& transform = registry.get<TransformComp>(entity);
        if(camera.isPrimary) {
            //update view ubo
            view = transform.getViewMatrix();
            projection = camera.projection;
            glm::mat4 vpMat = projection * view;
            auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
            m_Renderer->getResourceManager()->writeToUBO(m_Renderer->getViewUBO().uboID, bytes);
            //update global ubo, todo maybe use a light component
            GlobalUBO globalUBO {
                .lightDir = m_Scene->getLightDir(),
                .cameraPos = transform.localPosition,
                //.time = glfwGetTime(),
                .ambientColor = m_Scene->getAmbientColor(),
            };
            auto const globalBytes = std::bit_cast<std::array<std::byte,
                sizeof(globalUBO)>>(globalUBO);
            m_Renderer->getResourceManager()->writeToUBO(m_Renderer->getGlobalUBO().uboID, globalBytes);
        }
    }
}
void RenderSystem::renderDrawables(const entt::registry &registry) {
    //todo sort by material
    auto viewDraw = registry.view<MeshDrawableComp, MaterialComp, TransformComp>();
    for(auto& entity : viewDraw) {
        //bind shader
        auto& material = viewDraw.get<MaterialComp>(entity);
        m_Renderer->getResourceManager()->bindShader(material.shaderID);

        //write to material ubo
        MaterialUBO materialUBO {
            .specularAndShininess = {material.specular, float(material.shininess)}
        };
        auto const materialBytes = std::bit_cast<std::array<std::byte,
            sizeof(materialUBO)>>(materialUBO);
        m_Renderer->getResourceManager()->writeToUBO(m_Renderer->getMaterialUBO().uboID, materialBytes);

        //bind material texture
        m_Renderer->getResourceManager()->bindMaterial(material);
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = viewDraw.get<TransformComp>(entity);
        auto const bytes = std::bit_cast<std::array<std::byte,
            sizeof(transform.worldMatrix)>>(transform.worldMatrix);
        m_Renderer->getResourceManager()->writeToUBO(m_Renderer->getInstanceUBO().uboID, bytes);

        //todo submit mesh to renderer
        MeshDrawableComp drawable = viewDraw.get<MeshDrawableComp>(entity);
        m_Renderer->getContext()->drawMesh(*m_Renderer->getResourceManager(), drawable);

    }
}

void RenderSystem::renderSkybox(const entt::registry &registry, const glm::mat4& view,
    const glm::mat4& projection) {
    auto skyboxView = registry.view<SkyboxComp, MeshDrawableComp>();
    for (auto entity: skyboxView) {
        m_Renderer->getContext()->depthMask(false);
        m_Renderer->getContext()->setDepthMode(LEQUAL);
        glm::mat4 viewCube = glm::mat4(glm::mat3(view));
        glm::mat4 vp = projection * viewCube;
        auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vp)>>(vp);
        m_Renderer->getResourceManager()->writeToUBO(m_Renderer->getViewUBO().uboID, bytes);
        //bind shader
        auto& skyboxComp = skyboxView.get<SkyboxComp>(entity);
        m_Renderer->getResourceManager()->bindShader(skyboxComp.shaderID);

        //bind cubemap texture
        m_Renderer->getResourceManager()->bindCubeMapTexture(skyboxComp.textureID);

        auto& drawable = skyboxView.get<MeshDrawableComp>(entity);
        m_Renderer->getContext()->drawMesh(*m_Renderer->getResourceManager(), drawable);

        m_Renderer->getContext()->depthMask(true);
        m_Renderer->getContext()->setDepthMode(LESS);
    }
}

void RenderSystem::bindSceneUBOs() {
    //bind uniform vp matrix
    m_Renderer->getResourceManager()->bindUBO(m_Renderer->getViewUBO().uboID);
    //bind instance ubo (different binding so its ok)
    m_Renderer->getResourceManager()->bindUBO(m_Renderer->getInstanceUBO().uboID);
    //bind global ubo (different binding so its ok)
    m_Renderer->getResourceManager()->bindUBO(m_Renderer->getGlobalUBO().uboID);
    //bind material ubo (different binding so its ok)
    m_Renderer->getResourceManager()->bindUBO(m_Renderer->getMaterialUBO().uboID);
}


void RenderSystem::onRender(const entt::registry& registry) {
    //write camera data
    glm::mat4 view;
    glm::mat4 projection;
    writeCameraData(registry, view, projection);

    bindSceneUBOs();

    //render all meshes with drawable comps
    renderDrawables(registry);

    //draw skybox
    renderSkybox(registry, view, projection);
}
