#include "ZeusEngineCore/RenderSystem.h"
#include <ZeusEngineCore/InputEvents.h>
#include "ZeusEngineCore/AssetLibrary.h"
#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

RenderSystem::RenderSystem() :
    m_Renderer(&Application::get().getEngine()->getRenderer()),
    m_ResourceManager(Application::get().getEngine()->getRenderer().getResourceManager()),
    m_Scene(&Application::get().getEngine()->getScene()) {

    auto library = Project::getActive()->getAssetLibrary();

    m_CubeDrawable = GPUHandle<GPUMesh>(library->getCubeID());
    m_QuadDrawable = GPUHandle<GPUMesh>(library->getQuadID());

    ShaderData quadShaderData {
        .vertPath = "/shaders/screenQuad.vert",
        .fragPath = "/shaders/screenQuad.frag",
        .geoPath = ""
    };
    auto quadShaderID = library->createAsset(quadShaderData, "QuadShader");
    m_QuadShaderID = GPUHandle<GPUShader>(quadShaderID);

    ShaderData normalsShader {
        .vertPath = "/shaders/normal-visual.vert",
        .fragPath = "/shaders/normal-visual.frag",
        .geoPath = "/shaders/normal-visual.geom"
    };
    auto normalsShaderID = library->createAsset(normalsShader, "NormalsShader");
    m_NormalsShaderID = GPUHandle<GPUShader>(normalsShaderID);

}
void RenderSystem::updateWorldTransforms() {
    auto view = m_Scene->getEntities<TransformComp>();

    for (auto e : view) {
        auto &tc = e.getComponent<TransformComp>();
        glm::mat4 local = tc.getLocalMatrix();

        if (auto parentComp = e.tryGetComponent<ParentComp>()) {
            //if(auto tranformComp = parentComp->parent.tryGetComponent<TransformComp>()) {
            //    tc.worldMatrix = tranformComp->worldMatrix * local;
            //}
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
        if(!entity.hasComponent<MaterialComp>() && !entity.hasComponent<SkyboxComp>()) {

            auto library = Project::getActive()->getAssetLibrary();
            entity.addComponent<MaterialComp>(library->getDefaultMaterialID());
        }
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
    auto viewDraw = m_Scene->getEntities<MeshComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {

        auto& materialComp = entity.getComponent<MaterialComp>();
        auto* mat = materialComp.handle.get();
        auto library = Project::getActive()->getAssetLibrary();
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
        m_Renderer->m_ResourceManager->bindMaterial(matRaw);
        if (m_IrradianceMapID.get()) {
            m_Renderer->m_ResourceManager->bindCubeMapTexture(m_IrradianceMapID->drawableID, 5);
        }
        if (m_PrefilterMapID.get()) {
            m_Renderer->m_ResourceManager->bindCubeMapTexture(m_PrefilterMapID->drawableID, 6);
        }
        if (m_BRDFLUTID.get()) {
            m_Renderer->m_ResourceManager->bindTexture(m_BRDFLUTID->drawableID, 7);
        }
        //write to instance ubo (todo check if last mesh is same for instancing)

        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        auto drawable = entity.getComponent<MeshComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager,
            *m_ResourceManager->get<GPUMesh>(drawable.handle.id()));

    }
}

void RenderSystem::renderDrawablesToShader(uint32_t shaderID) {
    m_Renderer->m_ResourceManager->bindShader(shaderID);
    auto viewDraw = m_Scene->getEntities<MeshComp, MaterialComp, TransformComp>();
    for(auto entity : viewDraw) {
        //write to instance ubo (todo check if last mesh is same for instancing)
        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        auto drawable = entity.getComponent<MeshComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager,
            *m_ResourceManager->get<GPUMesh>(drawable.handle.id()));

    }
}

void RenderSystem::initSkyboxAssets(SkyboxComp& comp) {
    auto library = Project::getActive()->getAssetLibrary();
    //load env map
    //TODO do this automatically by specifying texture type and size


    TextureData skyboxData {
        .type = CubemapHDR,
        .dimensions = glm::vec2(1024, 1024),
        .mip = true,
    };
    auto skyboxTex = AssetHandle<TextureData>(std::move(skyboxData), "Skybox");

    TextureData hdrData = {
        .path = "/env-maps/christmas_photo_studio_04_4k.hdr",
        .type = HDR,
        .mip = false,
        .absPath = false,
    };
    auto hdrTex = AssetHandle<TextureData>(std::move(hdrData), "HDRTex");

    TextureData conData = {
        .type = CubemapHDR,
        .dimensions = glm::vec2(32, 32),
        .mip = false,
    };
    auto conTex = AssetHandle<TextureData>(std::move(conData), "Convuluted Map");
    m_IrradianceMapID = GPUHandle<GPUTexture>(conTex.id());

    TextureData prefilterData = {
        .type = Prefilter,
        .dimensions = glm::vec2(128, 128),
    };
    auto prefilterTex = AssetHandle<TextureData>( std::move(prefilterData), "Prefilter Map");
    m_PrefilterMapID = GPUHandle<GPUTexture>(prefilterTex.id());

    TextureData brdfData = {
        .type = BRDF,
        .dimensions = glm::vec2(1024, 1024),
    };
    auto brdfTex = AssetHandle<TextureData>(std::move(brdfData),"BRDF Map");
    m_BRDFLUTID = GPUHandle<GPUTexture>(brdfTex.id());

    AssetHandle skyboxShader = ShaderData("/shaders/glskyboxHDR.vert", "/shaders/glskyboxHDR.frag", "");
    Material skyboxMat {
        .shader = skyboxShader.id(),
        .texture = skyboxTex.id(),
    };

    AssetHandle eqToCubeMapShader = ShaderData("/shaders/eq-to-cubemap.vert", "/shaders/eq-to-cubemap.frag", "");
    Material eqMap {
        .shader = eqToCubeMapShader.id(),
        .texture = hdrTex.id(),
    };


    AssetHandle irradianceShader = ShaderData("/shaders/irradiance-con.vert", "/shaders/irradiance-con.frag", "");
    Material conMap {
        .shader = irradianceShader.id(),
        .texture = conTex.id(),
    };


    AssetHandle prefilterShader = ShaderData("/shaders/prefilter.vert", "/shaders/prefilter.frag", "");
    Material prefilterMap {
        .shader = prefilterShader.id(),
        .texture = prefilterTex.id(),

    };

    AssetHandle brdfShader = ShaderData("/shaders/brdf-con.vert", "/shaders/brdf-con.frag", "");
    Material brdfLUT {
        .shader = brdfShader.id(),
        .texture = brdfTex.id(),

    };

    SkyboxComp skybox {
        .skyboxMat = AssetHandle<Material>( std::move(skyboxMat), "SkyboxMat"),
        .eqMat = AssetHandle<Material>(std::move(eqMap), "EqMat"),
        .conMat = AssetHandle<Material>( std::move(conMap), "ConMat"),
        .prefilterMat = AssetHandle<Material>( std::move(prefilterMap), "PrefilterMat"),
        .brdfLUTMat = AssetHandle<Material>( std::move(brdfLUT), "BRDFMat"),

    };
    comp = skybox;

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

            m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxMat.textureID, 0);
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

        m_Renderer->m_ResourceManager->bindShader(skyboxMat.shaderID);
        m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxMat.textureID, 0);

        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager,
            *m_ResourceManager->get<GPUMesh>(drawable.handle.id()));

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
