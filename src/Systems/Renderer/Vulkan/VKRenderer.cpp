#include "ZeusEngineCore/engine/rendering/VKRenderer.h"
#include "ZeusEngineCore/engine/rendering/VKBackend.h"
#include "ZeusEngineCore/engine/rendering/VKContext.h"

using namespace ZEN;

VKRenderer::VKRenderer() {

}

void VKRenderer::init(EngineContext* ctx) {
    m_StateCtx.init();

    m_RenderCtx.init(&m_StateCtx, &m_ResourceCtx);
    m_ResourceCtx.init(&m_StateCtx, &m_RenderCtx);

    m_RenderCtx.m_SkyboxRenderer->init("/env-maps/HDR_029_Sky_Cloudy_Ref.hdr");
}

void VKRenderer::beginFrame() {
    m_RenderCtx.beginFrame();
}

void VKRenderer::draw() {
    m_RenderCtx.draw();
}

void VKRenderer::endFrame() {
    m_RenderCtx.endFrame();
}

VKRenderer::~VKRenderer() {
}

ImGui_ImplVulkan_InitInfo VKRenderer::initImgui() {
    return m_ResourceCtx.initImgui();
}

void VKRenderer::cleanup() {
    m_RenderCtx.cleanup();
    m_ResourceCtx.cleanup();
    m_StateCtx.cleanup();
}

void VKRenderer::submitDrawCall(const DrawCall &call) {
    m_RenderCtx.m_DrawCalls.push_back(call);
}

void VKRenderer::setImGUIMode(const bool mode) {
    m_RenderCtx.m_RenderToIMGUITexture = mode;
}




