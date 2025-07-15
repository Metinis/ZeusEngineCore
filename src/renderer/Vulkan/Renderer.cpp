#include "Renderer.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace ZEN::VKAPI;

void Renderer::Init(RendererInitInfo& initInfo) {
    if(!initInfo.windowHandle.nativeWindowHandle)
        throw std::runtime_error("Null window handle!");

    //todo create Interface for this
    m_Backend = std::make_unique<VKAPI::APIBackend>(initInfo.windowHandle);
    m_APIRenderer = std::make_unique<VKAPI::APIRenderer>(m_Backend.get());
    m_ViewUBO.emplace(m_Backend->CreateUBO());
    ZEN::TextureInfo textureInfo{};
    textureInfo.textInfoVariant = m_Backend->GetTextureInfo();
    m_Texture.Init(textureInfo);
}

Renderer::~Renderer() = default;


bool Renderer::BeginFrame() {
    if(!m_APIRenderer->BeginFrame()){
        return false;
    }
    UpdateView();
    return true;
}

void Renderer::Submit(const glm::mat4& transform, const std::shared_ptr<Material>& material, const std::shared_ptr<IMesh>& mesh) {
    m_RenderQueue.emplace_back(transform, material, mesh);

}


void Renderer::EndFrame(const std::function<void(void*)>& uiExtraDrawCallback) {
    m_APIRenderer->SetUBO(m_ViewUBO.value());
    for(const auto& cmd : m_RenderQueue){
        m_APIRenderer->SetImage(m_Texture);
        m_APIRenderer->BindDescriptorSets();
        cmd.material->Bind();
        cmd.mesh->Draw(*cmd.material); //placeholder, later just retrieve draw data
        //call APIRenderer directly here
    }
    m_RenderQueue.clear();
    if(uiExtraDrawCallback){
        m_APIRenderer->DrawWithCallback(uiExtraDrawCallback); //injects command buffer
    }
    m_APIRenderer->SubmitAndPresent();
}


ZEN::RendererContextVariant Renderer::GetContext() const
{
    ContextInfo contextInfo = m_Backend->GetContext();
    return contextInfo;
}

ZEN::ShaderInfoVariant Renderer::GetShaderInfo() const {
    return m_Backend->GetShaderInfo();
}

void Renderer::UpdateView()
{
    auto const halfSize = 0.5f * glm::vec2{m_Backend->GetFramebufferSize()};
    auto const matProjection = glm::ortho(-halfSize.x, halfSize.x, -halfSize.y, halfSize.y);
    auto const bytes = std::bit_cast<std::array<std::byte, sizeof(matProjection)>>(matProjection);
    m_ViewUBO->WriteAt(m_APIRenderer->GetFrameIndex(), bytes);
}

APIRenderer* Renderer::GetAPIRenderer() const {
    return m_APIRenderer.get();
}
