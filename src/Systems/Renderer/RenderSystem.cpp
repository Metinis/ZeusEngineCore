#include "ZeusEngineCore/RenderSystem.h"

#include <ZeusEngineCore/InputEvents.h>

#include "ZeusEngineCore/ModelLibrary.h"

#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

RenderSystem::RenderSystem(Renderer *renderer, Scene *scene, ModelLibrary* library,
            EventDispatcher* dispatcher) :
m_Renderer(renderer), m_Scene(scene), m_Library(library), m_Dispatcher(dispatcher){
    m_Dispatcher->attach<RemoveMeshEvent, RenderSystem, &RenderSystem::onMeshRemove>(this);
    m_Dispatcher->attach<RemoveMeshCompEvent, RenderSystem, &RenderSystem::onMeshCompRemove>(this);
    m_Dispatcher->attach<RemoveMeshDrawableEvent, RenderSystem, &RenderSystem::onMeshDrawableRemove>(this);

}
void RenderSystem::updateWorldTransforms() {
    auto view = m_Scene->getEntities<TransformComp>();

    for (auto e : view) {
        auto &tc = e.getComponent<TransformComp>();
        glm::mat4 local = tc.getLocalMatrix();

        if (auto parentComp = e.tryGetComponent<ParentComp>()) {
            if(auto tranformComp = parentComp->parent.tryGetComponent<TransformComp>()) {
                tc.worldMatrix = tranformComp->worldMatrix * local;
            }
        }
        else {
            tc.worldMatrix = local;
        }
    }
}

void RenderSystem::onMeshRemove(RemoveMeshEvent &e) {
    //remove the drawable comp here
    auto view = m_Scene->getEntities<MeshDrawableComp>();
    for(auto entity : view) {
        if(entity.getComponent<MeshDrawableComp>().name == e.meshName) {
            entity.removeComponent<MeshDrawableComp>();
        }
    }
}

void RenderSystem::onMeshCompRemove(RemoveMeshCompEvent &e) {
    if(e.entity.hasComponent<MeshDrawableComp>()) {
        e.entity.removeComponent<MeshDrawableComp>();
    }

}

void RenderSystem::onMeshDrawableRemove(RemoveMeshDrawableEvent &e) {
    m_Renderer->getResourceManager()->deleteMeshDrawable(
            e.entity.getComponent<MeshDrawableComp>().meshID);
}

void RenderSystem::onUpdate() {
    updateWorldTransforms();
    //create buffers for all meshes without drawable comps
    auto meshView = m_Scene->getEntities<MeshComp>();
    for (auto entity : meshView) {
        if(entity.hasComponent<MeshDrawableComp>()) continue;

        auto& meshComp = entity.getComponent<MeshComp>();
        if(meshComp.name.empty()) continue;
        auto mesh = m_Library->getMesh(meshComp.name);
        if(!mesh) continue;

        if(!entity.hasComponent<MaterialComp>() && !entity.hasComponent<SkyboxComp>()) {
            entity.addComponent<MaterialComp>(MaterialComp{ .name = "Default"}); //default needs to exist in model library
        }

        MeshDrawableComp drawableComp{};
        drawableComp.name = meshComp.name;
        drawableComp.indexCount = mesh->indices.size();
        drawableComp.meshID = m_Renderer->m_ResourceManager->createMeshDrawable(*mesh);
        entity.addComponent<MeshDrawableComp>(drawableComp);
    }


}
void RenderSystem::writeCameraData(glm::mat4& view, glm::mat4& projection) {
    auto cameraView = m_Scene->getEntities<CameraComp, TransformComp>();
    for (auto entity: cameraView) {
        auto& camera = entity.getComponent<CameraComp>();
        auto& transform = entity.getComponent<TransformComp>();
        if(camera.isPrimary) {
            //update view ubo
            view = transform.getViewMatrix();
            projection = camera.projection;
            glm::mat4 vpMat = projection * view;
            auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vpMat)>>(vpMat);
            m_Renderer->m_ResourceManager->writeToUBO(m_Renderer->m_ViewUBO.uboID, bytes);
            //update global ubo, todo maybe use a light component
            GlobalUBO globalUBO {
                .lightDir = m_Scene->getLightDir(),
                .cameraPos = transform.localPosition,
                //.time = glfwGetTime(),
                .ambientColor = m_Scene->getAmbientColor(),
            };
            auto const globalBytes = std::bit_cast<std::array<std::byte,
                sizeof(globalUBO)>>(globalUBO);
            m_Renderer->m_ResourceManager->writeToUBO(m_Renderer->m_GlobalUBO.uboID, globalBytes);
        }
    }
}
void RenderSystem::renderDrawables() {
    //todo sort by material
    auto viewDraw = m_Scene->getEntities<MeshDrawableComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {
        //bind shader
        auto& materialComp = entity.getComponent<MaterialComp>();
        auto material = m_Library->getMaterial(materialComp.name);
        m_Renderer->m_ResourceManager->bindShader(material->shaderID);

        //write to material ubo
        MaterialUBO materialUBO {
            .specularAndShininess = {material->specular, float(material->shininess)}
        };
        auto const materialBytes = std::bit_cast<std::array<std::byte,
            sizeof(materialUBO)>>(materialUBO);
        m_Renderer->m_ResourceManager->writeToUBO(m_Renderer->m_MaterialUBO.uboID, materialBytes);

        //bind material texture
        m_Renderer->m_ResourceManager->bindMaterial(*material);
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = entity.getComponent<TransformComp>();
        auto const bytes = std::bit_cast<std::array<std::byte,
            sizeof(transform.worldMatrix)>>(transform.worldMatrix);
        m_Renderer->m_ResourceManager->writeToUBO(m_Renderer->m_InstanceUBO.uboID, bytes);

        //todo submit mesh to renderer
        MeshDrawableComp drawable = entity.getComponent<MeshDrawableComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, drawable);

    }
}

void RenderSystem::renderSkybox(const glm::mat4& view, const glm::mat4& projection) {
    auto skyboxView = m_Scene->getEntities<SkyboxComp, MeshDrawableComp>();
    for (auto entity: skyboxView) {
        m_Renderer->m_Context->depthMask(false);
        m_Renderer->m_Context->setDepthMode(LEQUAL);
        glm::mat4 viewCube = glm::mat4(glm::mat3(view));
        glm::mat4 vp = projection * viewCube;
        auto const bytes = std::bit_cast<std::array<std::byte, sizeof(vp)>>(vp);
        m_Renderer->m_ResourceManager->writeToUBO(m_Renderer->m_ViewUBO.uboID, bytes);
        //bind shader
        auto& skyboxComp = entity.getComponent<SkyboxComp>();
        m_Renderer->m_ResourceManager->bindShader(skyboxComp.shaderID);

        //bind cubemap texture
        m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxComp.textureID);

        auto& drawable = entity.getComponent<MeshDrawableComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, drawable);

        m_Renderer->m_Context->depthMask(true);
        m_Renderer->m_Context->setDepthMode(LESS);
    }
}

void RenderSystem::bindSceneUBOs() {
    //bind uniform vp matrix
    m_Renderer->m_ResourceManager->bindUBO(m_Renderer->m_ViewUBO.uboID);
    //bind instance ubo (different binding so its ok)
    m_Renderer->m_ResourceManager->bindUBO(m_Renderer->m_InstanceUBO.uboID);
    //bind global ubo (different binding so its ok)
    m_Renderer->m_ResourceManager->bindUBO(m_Renderer->m_GlobalUBO.uboID);
    //bind material ubo (different binding so its ok)
    m_Renderer->m_ResourceManager->bindUBO(m_Renderer->m_MaterialUBO.uboID);
}


void RenderSystem::onRender() {
    //write camera data
    glm::mat4 view;
    glm::mat4 projection;
    writeCameraData(view, projection);

    bindSceneUBOs();

    //render all meshes with drawable comps
    renderDrawables();

    //draw skybox
    renderSkybox(view, projection);
}
