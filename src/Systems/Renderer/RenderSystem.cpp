#include "ZeusEngineCore/RenderSystem.h"
#include <ZeusEngineCore/InputEvents.h>
#include "ZeusEngineCore/ModelLibrary.h"
#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

RenderSystem::RenderSystem(Renderer *renderer, Scene *scene, ModelLibrary* library) :
m_Renderer(renderer), m_Scene(scene), m_Library(library){

    //m_Dispatcher->attach<RemoveMeshEvent, RenderSystem, &RenderSystem::onMeshRemove>(this);
    //m_Dispatcher->attach<RemoveMeshCompEvent, RenderSystem, &RenderSystem::onMeshCompRemove>(this);
    //m_Dispatcher->attach<RemoveMeshDrawableEvent, RenderSystem, &RenderSystem::onMeshDrawableRemove>(this);
    //m_Dispatcher->attach<ToggleDrawNormalsEvent, RenderSystem, &RenderSystem::onToggleDrawNormals>(this);

    m_CubeDrawable = *m_Library->getMeshDrawable("Cube");
    m_QuadDrawable = *m_Library->getMeshDrawable("Quad");
    m_QuadShaderID = m_Library->getMaterial("ScreenQuad")->shaderID;

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

/*void RenderSystem::onMeshRemove(RemoveMeshEvent &e) {
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

void RenderSystem::onToggleDrawNormals(ToggleDrawNormalsEvent &e) {
    m_DrawNormals = !m_DrawNormals;
}*/

void RenderSystem::onUpdate(float deltaTime) {
    updateWorldTransforms();
    //create buffers for all meshes without drawable comps
    auto meshView = m_Scene->getEntities<MeshComp>();
    for (auto entity : meshView) {
        if(entity.hasComponent<MeshDrawableComp>()) continue;

        auto& meshComp = entity.getComponent<MeshComp>();
        if(meshComp.name.empty()) continue;
        auto mesh = m_Library->getMeshData(meshComp.name);
        if(!mesh) continue;

        if(!entity.hasComponent<MaterialComp>() && !entity.hasComponent<SkyboxComp>()) {
            entity.addComponent<MaterialComp>(MaterialComp{ .name = "Default"}); //default needs to exist in model library
        }
        MeshDrawableComp comp{};

        if(!m_Library->hasDrawable(meshComp.name)) {
            m_Library->createAndAddDrawable(meshComp.name, *m_Library->getMeshData(meshComp.name)); //todo create a overload for just name
        }
        comp.name = meshComp.name;
        entity.addComponent<MeshDrawableComp>(comp);
        //todo have the drawable in the modellibrary to avoid creating the same drawable
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

            m_Renderer->writeToUBO(m_Renderer->m_ViewUBO.uboID, vpMat);

            setLightData(transform.localPosition);

        }
    }
}

void RenderSystem::setLightData(glm::vec3 cameraPos) {
    auto lightView = m_Scene->getEntities<DirectionalLightComp, TransformComp>();

    for(auto entity : lightView) {
        auto& dirLight = entity.getComponent<DirectionalLightComp>();

        if(!dirLight.isPrimary) {
            continue;
        }

        auto& transform = entity.getComponent<TransformComp>();
        GlobalUBO globalUBO {
            .lightPos = transform.localPosition,
            .cameraPos = cameraPos,
            .ambientColor = dirLight.ambient,
        };
        m_Renderer->writeToUBO(m_Renderer->m_GlobalUBO.uboID, globalUBO);
    }

}

void RenderSystem::renderDrawables() {
    //todo sort by material
    auto viewDraw = m_Scene->getEntities<MeshDrawableComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {

        auto& materialComp = entity.getComponent<MaterialComp>();
        auto material = m_Library->getMaterial(materialComp.name);
        //m_Renderer->m_ResourceManager->bindShader(material->shaderID);

        //convert to UBO format
        glm::vec4 alb;
        alb.x = material->albedo.x;
        alb.y = material->albedo.y;
        alb.z = material->albedo.z;

        glm::vec4 props;
        props.x = material->metallic;
        props.y = material->roughness;
        props.z = material->ao;
        props.w = material->metal;
        MaterialUBO materialUBO {
            .albedo = alb,
            .params = props
        };

        m_Renderer->writeToUBO(m_Renderer->m_MaterialUBO.uboID, materialUBO);

        //bind material texture
        m_Renderer->m_ResourceManager->bindMaterial(*material);
        m_Renderer->m_ResourceManager->bindCubeMapTexture(m_IrradianceMapID, 5);
        m_Renderer->m_ResourceManager->bindCubeMapTexture(m_PrefilterMapID, 6);
        m_Renderer->m_ResourceManager->bindTexture(m_BRDFLUTID, 7);
        //write to instance ubo (todo check if last mesh is same for instancing)

        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        MeshDrawableComp drawable = entity.getComponent<MeshDrawableComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, *m_Library->getMeshDrawable(drawable.name));

    }
}

void RenderSystem::renderDrawablesToShader(uint32_t shaderID) {
    m_Renderer->m_ResourceManager->bindShader(shaderID);
    auto viewDraw = m_Scene->getEntities<MeshDrawableComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        MeshDrawableComp drawable = entity.getComponent<MeshDrawableComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, *m_Library->getMeshDrawable(drawable.name));

    }
}

void RenderSystem::renderSkybox(const glm::mat4& view, const glm::mat4& projection) {

    auto skyboxView = m_Scene->getEntities<SkyboxComp, MeshDrawableComp>();
    for (auto entity: skyboxView) {
        auto& drawable = entity.getComponent<MeshDrawableComp>();
        auto& skyboxComp = entity.getComponent<SkyboxComp>();

        auto skyboxMat = m_Library->getMaterial(skyboxComp.skyboxMat.name);
        auto eqMat = m_Library->getMaterial(skyboxComp.eqMat.name);
        auto conMat = m_Library->getMaterial(skyboxComp.conMat.name);
        auto prefilterMat = m_Library->getMaterial(skyboxComp.prefilterMat.name);
        auto brdfLUTMat = m_Library->getMaterial(skyboxComp.brdfLUTMat.name);


        if(!skyboxComp.envGenerated) {
            m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxMat->textureID, 0);
            m_Renderer->renderToCubeMapHDR(skyboxMat->textureID, eqMat->shaderID, eqMat->textureID,
                m_CubeDrawable);

            m_Renderer->renderToIrradianceMap(skyboxMat->textureID, conMat->textureID,
                conMat->shaderID, m_CubeDrawable);

            m_IrradianceMapID = conMat->textureID;

            m_Renderer->renderToPrefilterMap(skyboxMat->textureID, prefilterMat->textureID,
                prefilterMat->shaderID, m_CubeDrawable);

            m_PrefilterMapID = prefilterMat->textureID;

            m_Renderer->renderToBRDFLUT(brdfLUTMat->textureID, brdfLUTMat->shaderID, m_QuadDrawable);
            m_BRDFLUTID = brdfLUTMat->textureID;

            skyboxComp.envGenerated = true;
        }


        m_Renderer->m_Context->depthMask(false);
        m_Renderer->m_Context->setDepthMode(LEQUAL);
        glm::mat4 viewCube = glm::mat4(glm::mat3(view));
        glm::mat4 vp = projection * viewCube;

        m_Renderer->writeToUBO(m_Renderer->m_ViewUBO.uboID, vp);

        m_Renderer->m_ResourceManager->bindShader(skyboxMat->shaderID);
        m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxMat->textureID, 0);

        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, *m_Library->getMeshDrawable(drawable.name));

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


bool RenderSystem::onPlayModeEvent(RunPlayModeEvent &e) {
    m_IsPlaying = e.getPlaying();
    return false;
}
void RenderSystem::onRender() {
    //write camera data
    glm::mat4 view;
    glm::mat4 projection;

    writeCameraData(view, projection);

    bindSceneUBOs();

    //render all meshes with drawable comps

    renderDrawables();

    if(m_DrawNormals) {
        renderDrawablesToShader(m_Library->getMaterial("NormalsMat")->shaderID);
    }

    //draw skybox
    renderSkybox(view, projection);

    if(m_IsPlaying) {
        //bind default FBO, render to screen quad
        m_Renderer->renderToScreenQuad(m_QuadShaderID, m_QuadDrawable);
    }

}

void RenderSystem::onEvent(Event &event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayModeEvent(e); });
}
