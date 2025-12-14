#include "ZeusEngineCore/RenderSystem.h"
#include <ZeusEngineCore/InputEvents.h>
#include "ZeusEngineCore/AssetLibrary.h"
#include "ZeusEngineCore/Scene.h"

using namespace ZEN;

RenderSystem::RenderSystem(Renderer *renderer, Scene *scene) :
m_Renderer(renderer), m_ResourceManager(renderer->getResourceManager()), m_Scene(scene) {
    auto library = Project::getActive()->getAssetLibrary();
    MeshData* cubeData = library->get<MeshData>(library->getCubeID());
    MeshDrawable cubeDrawable {
        .drawableID = m_Renderer->m_ResourceManager->createMeshDrawable(*cubeData),
        .indexCount = cubeData->indices.size(),
        1
    };
    m_CubeDrawable = AssetHandle<MeshDrawable>(std::move(cubeDrawable));

    MeshData* quadData = library->get<MeshData>(library->getQuadID());
    MeshDrawable quadDrawable {
        .drawableID = m_Renderer->m_ResourceManager->createMeshDrawable(*quadData),
        .indexCount = cubeData->indices.size(),
        1
    };
    m_QuadDrawable = AssetHandle<MeshDrawable>(std::move(quadDrawable));

    ShaderData quadShaderData {
        .vertPath = "/shaders/screenQuad.vert",
        .fragPath = "/shaders/screenQuad.frag",
        .geoPath = ""
    };

    m_QuadShaderID = AssetHandle<ShaderData>(std::move(quadShaderData));

    ShaderData normalsShader {
        .vertPath = "/shaders/normal-visual.vert",
        .fragPath = "/shaders/normal-visual.frag",
        .geoPath = "/shaders/normal-visual.geom"
    };
    m_NormalsShaderID = AssetHandle<ShaderData>(std::move(normalsShader));

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
        if(entity.hasComponent<MeshDrawableComp>()) continue;

        auto& meshComp = entity.getComponent<MeshComp>();
        //if(!meshComp.handle.) continue;

        if(!entity.hasComponent<MaterialComp>() && !entity.hasComponent<SkyboxComp>()) {

            auto library = Project::getActive()->getAssetLibrary();
            entity.addComponent<MaterialComp>(MaterialComp{.handle =
                AssetHandle<Material>(library->getDefaultMaterialID())});
        }

        MeshDrawable drawable {
            .drawableID = m_Renderer->m_ResourceManager->createMeshDrawable(*meshComp.handle.get()),
            .indexCount = meshComp.handle.get()->indices.size(),
            1
        };
        auto meshDrawable = AssetHandle<MeshDrawable>(std::move(drawable));
        entity.addComponent<MeshDrawableComp>(meshDrawable);
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
        m_Renderer->m_ResourceManager->bindCubeMapTexture(m_IrradianceMapID.get()->id, 5);
        m_Renderer->m_ResourceManager->bindCubeMapTexture(m_PrefilterMapID.get()->id, 6);
        m_Renderer->m_ResourceManager->bindTexture(m_BRDFLUTID.get()->id, 7);
        //write to instance ubo (todo check if last mesh is same for instancing)

        auto transform = entity.getComponent<TransformComp>();

        m_Renderer->writeToUBO(m_Renderer->m_InstanceUBO.uboID, transform.worldMatrix);

        //todo submit mesh to renderer
        auto drawable = entity.getComponent<MeshDrawableComp>();
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, *drawable.handle.get());

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
        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, *drawable.handle.get());

    }
}

void RenderSystem::initSkyboxAssets(SkyboxComp& comp) {
    auto library = Project::getActive()->getAssetLibrary();
    //load env map
    TextureData skyboxData {
        .id = m_ResourceManager->createCubeMapTextureHDRMip(1024, 1024)
    };
    auto skyboxTex = AssetHandle<TextureData>(UUID(), std::move(skyboxData));

    TextureData hdrData = {
        .id = m_ResourceManager->createHDRTexture("/env-maps/christmas_photo_studio_04_4k.hdr")
    };
    auto hdrTex = AssetHandle<TextureData>(UUID(), std::move(hdrData));

    TextureData conData = {
        .id = m_ResourceManager->createCubeMapTextureHDR(32, 32)
    };
    auto conTex = AssetHandle<TextureData>(UUID(), std::move(conData));

    TextureData prefilterData = {
        .id = m_ResourceManager->createPrefilterMap(128, 128)
    };
    auto prefilterTex = AssetHandle<TextureData>(UUID(), std::move(prefilterData));

    TextureData brdfData = {
        .id = m_ResourceManager->createBRDFLUTTexture(1024, 1024)
    };
    auto brdfTex = AssetHandle<TextureData>(UUID(), std::move(brdfData));


    auto skyboxShader = ShaderData("/shaders/glskyboxHDR.vert", "/shaders/glskyboxHDR.frag", "");
    auto shader = AssetHandle<ShaderData>(std::move(skyboxShader));
    Material skyboxMat {
        .shader = shader.id(),
        .texture = skyboxTex.id(),
    };

    auto eqToCubeMapShader = ShaderData("/shaders/eq-to-cubemap.vert", "/shaders/eq-to-cubemap.frag", "");
    shader = AssetHandle<ShaderData>(std::move(eqToCubeMapShader));
    Material eqMap {
        .shader = shader.id(),
        .texture = hdrTex.id(),
    };


    auto irradianceShader = ShaderData("/shaders/irradiance-con.vert", "/shaders/irradiance-con.frag", "");
    shader = AssetHandle<ShaderData>(std::move(irradianceShader));
    Material conMap {
        .shader = shader.id(),
        .texture = conTex.id(),
    };


    auto prefilterShader = ShaderData("/shaders/prefilter.vert", "/shaders/prefilter.frag", "");
    shader = AssetHandle<ShaderData>(std::move(prefilterShader));
    Material prefilterMap {
        .shader = shader.id(),
        .texture = prefilterTex.id(),

    };

    auto brdfShader = ShaderData("/shaders/brdf-con.vert", "/shaders/brdf-con.frag", "");
    shader = AssetHandle<ShaderData>( std::move(brdfShader));
    Material brdfLUT {
        .shader = shader.id(),
        .texture = brdfTex.id(),

    };

    SkyboxComp skybox {
        .skyboxMat = AssetHandle<Material>(UUID(), std::move(skyboxMat)),
        .eqMat = AssetHandle<Material>(UUID(), std::move(eqMap)),
        .conMat = AssetHandle<Material>(UUID(), std::move(conMap)),
        .prefilterMat = AssetHandle<Material>(UUID(), std::move(prefilterMap)),
        .brdfLUTMat = AssetHandle<Material>(UUID(), std::move(brdfLUT)),

    };
    comp = skybox;

}
void RenderSystem::renderSkybox(const glm::mat4& view, const glm::mat4& projection) {
    auto skyboxView = m_Scene->getEntities<SkyboxComp, MeshDrawableComp>();
    for (auto entity: skyboxView) {
        auto& drawable = entity.getComponent<MeshDrawableComp>();
        auto& skyboxComp = entity.getComponent<SkyboxComp>();
        if(!skyboxComp.envGenerated) {
            initSkyboxAssets(skyboxComp);
        }

        auto library = Project::getActive()->getAssetLibrary();
        auto skyboxMat = library->getMaterialRaw(*skyboxComp.skyboxMat.handle.get());
        auto eqMat = library->getMaterialRaw(*skyboxComp.eqMat.handle.get());
        auto conMat = library->getMaterialRaw(*skyboxComp.conMat.handle.get());
        auto prefilterMat = library->getMaterialRaw(*skyboxComp.prefilterMat.handle.get());
        auto brdfLUTMat = library->getMaterialRaw(*skyboxComp.brdfLUTMat.handle.get());


        if(!skyboxComp.envGenerated) {

            m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxMat.textureID, 0);
            m_Renderer->renderToCubeMapHDR(skyboxMat.textureID, eqMat.shaderID, eqMat.textureID,
                *m_CubeDrawable.get());

            m_Renderer->renderToIrradianceMap(skyboxMat.textureID, conMat.textureID,
                conMat.shaderID, *m_CubeDrawable.get());

            m_IrradianceMapID = AssetHandle<TextureData>(skyboxComp.conMat.handle.get()->texture);


            m_Renderer->renderToPrefilterMap(skyboxMat.textureID, prefilterMat.textureID,
                prefilterMat.shaderID, *m_CubeDrawable.get());

            m_PrefilterMapID = AssetHandle<TextureData>(skyboxComp.prefilterMat.handle.get()->texture);

            m_Renderer->renderToBRDFLUT(brdfLUTMat.textureID, brdfLUTMat.shaderID, *m_QuadDrawable.get());
            m_PrefilterMapID = AssetHandle<TextureData>(skyboxComp.brdfLUTMat.handle.get()->texture);

            skyboxComp.envGenerated = true;
        }


        m_Renderer->m_Context->depthMask(false);
        m_Renderer->m_Context->setDepthMode(LEQUAL);
        glm::mat4 viewCube = glm::mat4(glm::mat3(view));
        glm::mat4 vp = projection * viewCube;

        m_Renderer->writeToUBO(m_Renderer->m_ViewUBO.uboID, vp);

        m_Renderer->m_ResourceManager->bindShader(skyboxMat.shaderID);
        m_Renderer->m_ResourceManager->bindCubeMapTexture(skyboxMat.textureID, 0);

        m_Renderer->m_Context->drawMesh(*m_Renderer->m_ResourceManager, *drawable.handle.get());

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
        renderDrawablesToShader(m_NormalsShaderID.get()->id);
    }

    //draw skybox
    renderSkybox(view, projection);

    if(m_IsPlaying) {
        //bind default FBO, render to screen quad
        m_Renderer->renderToScreenQuad(m_QuadShaderID.get()->id, *m_QuadDrawable.get());
    }

}

void RenderSystem::onEvent(Event &event) {
    EventDispatcher dispatcher(event);

    dispatcher.dispatch<RunPlayModeEvent>([this](RunPlayModeEvent& e) {return onPlayModeEvent(e); });
}
