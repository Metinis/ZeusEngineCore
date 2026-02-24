#include "ZeusEngineCore/engine/RenderSystem.h"
#include <ZeusEngineCore/core/InputEvents.h>
#include "ZeusEngineCore/asset/AssetLibrary.h"
#include "ZeusEngineCore/engine/CameraSystem.h"
#include "ZeusEngineCore/engine/Scene.h"

using namespace ZEN;

#ifdef USE_OPENGL
RenderSystem::RenderSystem() :
    m_Renderer(&Application::get().getEngine()->getRenderer()),
    m_ResourceManager(Application::get().getEngine()->getRenderer().getResourceManager()),
    m_Scene(&Application::get().getEngine()->getScene()) {

    auto library = Project::getActive()->getAssetLibrary();

    m_CubeDrawable = GPUHandle<GPUMesh>(defaultCubeID);
    m_QuadDrawable = GPUHandle<GPUMesh>(defaultQuadID);


    m_QuadShaderID = GPUHandle<GPUShader>(defaultQuadShaderID);
    m_NormalsShaderID = GPUHandle<GPUShader>(defaultNormalsShaderID);
    //initPicking();
}


void RenderSystem::onUpdate(float deltaTime) {
    //create buffers for all meshes without drawable comps
    auto meshView = m_Scene->getEntities<MeshComp>();
    for (auto entity : meshView) {
        if(!entity.hasComponent<MaterialComp>() && !entity.hasComponent<SkyboxComp>()) {

            auto library = Project::getActive()->getAssetLibrary();
            entity.addComponent<MaterialComp>(defaultMaterialID);
        }
    }
}
void RenderSystem::writeCameraData(glm::mat4& view, glm::mat4& projection) {
    if (!m_IsPlaying && !Application::get().getEngine()->getCameraSystem().getUseMainCamera()) {
        auto sceneCameraView = m_Scene->getEntities<SceneCameraComp, TransformComp>();
        for (auto entity: sceneCameraView) {
            auto& camera = entity.getComponent<SceneCameraComp>();
            auto& transform = entity.getComponent<TransformComp>();
            view = transform.getViewMatrixWorld();
            projection = camera.projection;
            glm::mat4 vpMat = projection * view;

            m_Renderer->writeToUBO(m_Renderer->m_ViewUBO.uboID, vpMat);

            setLightData(transform.getWorldPosition());
        }
    }
    else {
        auto cameraView = m_Scene->getEntities<CameraComp, TransformComp>();
        for (auto entity: cameraView) {
            auto& camera = entity.getComponent<CameraComp>();
            auto& transform = entity.getComponent<TransformComp>();
            view = transform.getViewMatrixWorld();
            projection = camera.projection;
            glm::mat4 vpMat = projection * view;

            m_Renderer->writeToUBO(m_Renderer->m_ViewUBO.uboID, vpMat);

            setLightData(transform.getWorldPosition());
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
            .lightPos = transform.getWorldPosition(),
            .cameraPos = cameraPos,
            .ambientColor = dirLight.ambient,
        };
        m_Renderer->writeToUBO(m_Renderer->m_GlobalUBO.uboID, globalUBO);
    }

}

void RenderSystem::renderDrawables() {
    //todo sort by material
    auto viewDraw = m_Scene->getEntities<MeshComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {

        auto& materialComp = entity.getComponent<MaterialComp>();
        auto* mat = materialComp.handle.get();
        auto library = Project::getActive()->getAssetLibrary();
        if (!mat) {
            materialComp.handle = defaultMaterialID;
            continue;
        }

        auto matRaw = library->getMaterialRaw(*mat);

        //convert to UBO format
        glm::vec4 alb;
        alb.x = matRaw.albedo.x;
        alb.y = matRaw.albedo.y;
        alb.z = matRaw.albedo.z;

        glm::vec4 props;
        props.x = matRaw.metallic;
        props.y = matRaw.roughness;
        props.z = matRaw.ao;
        props.w = matRaw.metal;
        MaterialUBO materialUBO {
            .albedo = alb,
            .params = props
        };

        m_Renderer->writeToUBO(m_Renderer->m_MaterialUBO.uboID, materialUBO);

        //bind material texture
        m_ResourceManager->bindMaterial(matRaw);
        if (m_IrradianceMapID.get()) {
            m_ResourceManager->bindCubeMapTexture(m_IrradianceMapID->drawableID, 5);
        }
        if (m_PrefilterMapID.get()) {
            m_ResourceManager->bindCubeMapTexture(m_PrefilterMapID->drawableID, 6);
        }
        if (m_BRDFLUTID.get()) {
            m_ResourceManager->bindTexture(m_BRDFLUTID->drawableID, 7);
        }
        //write to instance ubo (todo check if last mesh is same for instancing)

        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        auto drawable = entity.getComponent<MeshComp>();
        if (auto* gpuMesh = m_ResourceManager->get<GPUMesh>(drawable.handle.id())) {
            m_Renderer->m_Context->drawMesh(*m_ResourceManager,
            *gpuMesh);
        }

    }
}

void RenderSystem::renderDrawablesToShader(uint32_t shaderID) {
    m_ResourceManager->bindShader(shaderID);
    auto viewDraw = m_Scene->getEntities<MeshComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        auto drawable = entity.getComponent<MeshComp>();
        if (auto* gpuMesh = m_ResourceManager->get<GPUMesh>(drawable.handle.id())) {
            m_Renderer->m_Context->drawMesh(*m_ResourceManager,
            *gpuMesh);
        }

    }
}



void RenderSystem::initSkyboxAssets(SkyboxComp& comp) {
    auto library = Project::getActive()->getAssetLibrary();
    //load env map
    if (!comp.skyboxMat.handle.get()) {
        TextureData skyboxData {
            .type = CubemapHDR,
            .dimensions = glm::vec2(1024, 1024),
            .mip = true,
        };
        auto skyboxTex = AssetHandle<TextureData>(std::move(skyboxData), "Skybox");
        AssetHandle skyboxShader = ShaderData("/shaders/glskyboxHDR.vert", "/shaders/glskyboxHDR.frag", "");
        Material skyboxMat {
            .shader = skyboxShader.id(),
            .texture = skyboxTex.id(),
        };
        comp.skyboxMat = {AssetHandle<Material>( std::move(skyboxMat), "SkyboxMat")};
    }
    if (!comp.eqMat.handle.get()) {
        TextureData hdrData = {
            .path = "/env-maps/HDR_029_Sky_Cloudy_Ref.hdr",
            .type = HDR,
            .mip = false,
            .absPath = false,
        };
        auto hdrTex = AssetHandle<TextureData>(std::move(hdrData), "HDRTex");
        AssetHandle eqToCubeMapShader = ShaderData("/shaders/eq-to-cubemap.vert", "/shaders/eq-to-cubemap.frag", "");
        Material eqMap {
            .shader = eqToCubeMapShader.id(),
            .texture = hdrTex.id(),
        };
        comp.eqMat = {AssetHandle<Material>(std::move(eqMap), "EqMat")};
    }
    if (!comp.conMat.handle.get()) {
        TextureData conData = {
            .type = CubemapHDR,
            .dimensions = glm::vec2(32, 32),
            .mip = false,
        };
        auto conTex = AssetHandle<TextureData>(std::move(conData), "Convuluted Map");
        m_IrradianceMapID = GPUHandle<GPUTexture>(conTex.id());
        AssetHandle irradianceShader = ShaderData("/shaders/irradiance-con.vert", "/shaders/irradiance-con.frag", "");
        Material conMap {
            .shader = irradianceShader.id(),
            .texture = conTex.id(),
        };
        comp.conMat = {AssetHandle<Material>(std::move(conMap), "ConMat")};
    }
    else {
        m_IrradianceMapID  = GPUHandle<GPUTexture>(comp.conMat.handle->texture);
    }
    if (!comp.prefilterMat.handle.get()) {
        TextureData prefilterData = {
            .type = Prefilter,
            .dimensions = glm::vec2(128, 128),
        };
        auto prefilterTex = AssetHandle<TextureData>( std::move(prefilterData), "Prefilter Map");
        m_PrefilterMapID = GPUHandle<GPUTexture>(prefilterTex.id());
        AssetHandle prefilterShader = ShaderData("/shaders/prefilter.vert", "/shaders/prefilter.frag", "");
        Material prefilterMap {
            .shader = prefilterShader.id(),
            .texture = prefilterTex.id(),
        };
        comp.prefilterMat = {AssetHandle<Material>(std::move(prefilterMap), "PrefilterMat")};
    }
    else {
        m_PrefilterMapID = GPUHandle<GPUTexture>(comp.prefilterMat.handle->texture);
    }
    if (!comp.brdfLUTMat.handle.get()) {
        TextureData brdfData = {
            .type = BRDF,
            .dimensions = glm::vec2(1024, 1024),
        };
        auto brdfTex = AssetHandle<TextureData>(std::move(brdfData),"BRDF Map");
        m_BRDFLUTID = GPUHandle<GPUTexture>(brdfTex.id());
        AssetHandle brdfShader = ShaderData("/shaders/brdf-con.vert", "/shaders/brdf-con.frag", "");
        Material brdfLUT {
            .shader = brdfShader.id(),
            .texture = brdfTex.id(),
        };
        comp.brdfLUTMat = {AssetHandle<Material>(std::move(brdfLUT), "BRDFMat")};
    }
    else {
        m_BRDFLUTID = GPUHandle<GPUTexture>(comp.brdfLUTMat.handle->texture);
    }

}
void RenderSystem::renderSkybox(const glm::mat4& view, const glm::mat4& projection) {
    auto skyboxView = m_Scene->getEntities<SkyboxComp, MeshComp>();
    for (auto entity: skyboxView) {
        auto& drawable = entity.getComponent<MeshComp>();
        auto& skyboxComp = entity.getComponent<SkyboxComp>();
        if(!skyboxComp.envGenerated) {
            initSkyboxAssets(skyboxComp);
        }

        auto library = Project::getActive()->getAssetLibrary();
        auto skyboxMat = library->getMaterialRaw(*skyboxComp.skyboxMat.handle);
        auto eqMat = library->getMaterialRaw(*skyboxComp.eqMat.handle);
        auto conMat = library->getMaterialRaw(*skyboxComp.conMat.handle);
        auto prefilterMat = library->getMaterialRaw(*skyboxComp.prefilterMat.handle);
        auto brdfLUTMat = library->getMaterialRaw(*skyboxComp.brdfLUTMat.handle);


        if(!skyboxComp.envGenerated) {

            m_ResourceManager->bindCubeMapTexture(skyboxMat.textureID, 0);
            m_Renderer->renderToCubeMapHDR(skyboxMat.textureID, eqMat.shaderID, eqMat.textureID,
                *m_CubeDrawable);

            m_Renderer->renderToIrradianceMap(skyboxMat.textureID, conMat.textureID,
                conMat.shaderID, *m_CubeDrawable);

            ShaderData quadShaderData {
                .vertPath = "/shaders/screenQuad.vert",
                .fragPath = "/shaders/screenQuad.frag",
                .geoPath = ""
            };

            m_Renderer->renderToPrefilterMap(skyboxMat.textureID, prefilterMat.textureID,
                prefilterMat.shaderID, *m_CubeDrawable);

            m_Renderer->renderToBRDFLUT(brdfLUTMat.textureID, brdfLUTMat.shaderID, *m_QuadDrawable);

            skyboxComp.envGenerated = true;
        }


        m_Renderer->m_Context->depthMask(false);
        m_Renderer->m_Context->setDepthMode(LEQUAL);
        glm::mat4 viewCube = glm::mat4(glm::mat3(view));
        glm::mat4 vp = projection * viewCube;

        m_Renderer->writeToUBO(m_Renderer->m_ViewUBO.uboID, vp);

        m_ResourceManager->bindShader(skyboxMat.shaderID);
        m_ResourceManager->bindCubeMapTexture(skyboxMat.textureID, 0);

        if (auto* gpuMesh = m_ResourceManager->get<GPUMesh>(drawable.handle.id())) {
            m_Renderer->m_Context->drawMesh(*m_ResourceManager,
            *gpuMesh);
        }

        m_Renderer->m_Context->depthMask(true);
        m_Renderer->m_Context->setDepthMode(LESS);
    }
}

void RenderSystem::bindSceneUBOs() {
    //bind uniform vp matrix
    m_ResourceManager->bindUBO(m_Renderer->m_ViewUBO.uboID);
    //bind instance ubo (different binding so its ok)
    m_ResourceManager->bindUBO(m_Renderer->m_InstanceUBO.uboID);
    //bind global ubo (different binding so its ok)
    m_ResourceManager->bindUBO(m_Renderer->m_GlobalUBO.uboID);
    //bind material ubo (different binding so its ok)
    m_ResourceManager->bindUBO(m_Renderer->m_MaterialUBO.uboID);
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

    //m_ResourceManager->bindFBO(m_Renderer->m_PickingFBO.fboID);
    if (!m_IsPlaying)
        m_Renderer->drawToPicking();
    //renderDrawablesToShader(GPUHandle<GPUShader>(defaultPickingShaderID).get()->drawableID);

    //m_ResourceManager->bindFBO(m_Renderer->m_MainFBO.fboID);
    //render all meshes with drawable comps
    renderDrawables();

    if(m_DrawNormals) {
        renderDrawablesToShader(m_NormalsShaderID.get()->drawableID);
    }

    //draw skybox
    renderSkybox(view, projection);

    if(m_IsPlaying) {
        //bind default FBO, render to screen quad
        m_Renderer->renderToScreenQuad(m_QuadShaderID.get()->drawableID, *m_QuadDrawable.get());
    }

}

void RenderSystem::onEvent(Event &event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayModeEvent(e); });
}
#endif
#ifdef USE_VULKAN
RenderSystem::RenderSystem() {
}

void RenderSystem::onUpdate(float deltaTime) {
    Layer::onUpdate(deltaTime);
}

void RenderSystem::onRender() {
    Layer::onRender();
}

void RenderSystem::onEvent(Event &event) {
    Layer::onEvent(event);
}

void RenderSystem::initSkyboxAssets(SkyboxComp &comp) {
}

bool RenderSystem::onPlayModeEvent(RunPlayModeEvent &e) {
}

void RenderSystem::writeCameraData(glm::mat4 &view, glm::mat4 &projection) {
}

void RenderSystem::setLightData(glm::vec3 cameraPos) {
}

void RenderSystem::bindSceneUBOs() {
}

void RenderSystem::renderDrawables() {
}

void RenderSystem::renderDrawablesToShader(uint32_t shaderID) {
}

void RenderSystem::renderSkybox(const glm::mat4 &view, const glm::mat4 &projection) {
}
#endif
