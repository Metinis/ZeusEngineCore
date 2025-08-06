#include "ShaderPipeline.h"
#include "ZeusEngineCore/Utils.h"
#include "Backend/APIRenderer.h"
#include "Backend/PipelineBuilder.h"

using namespace ZEN::VKAPI;

ShaderPipeline::ShaderPipeline(const ZEN::VKAPI::ShaderInfo &shaderInfo){
    assert(shaderInfo.apiRenderer);
    m_APIRenderer = shaderInfo.apiRenderer;

    const auto vertexSpirv = ToSpirV(shaderInfo.vertexPath);
    const auto fragmentSpirv = ToSpirV(shaderInfo.fragmentPath);

    if (vertexSpirv.empty() || fragmentSpirv.empty()) {
        throw std::runtime_error{"Failed to load shaders"};
    }
    m_Waiter = shaderInfo.device;

    vk::ShaderModuleCreateInfo vertexCreateInfo{};
    vertexCreateInfo.setCode(vertexSpirv);

    vk::ShaderModuleCreateInfo fragmentCreateInfo{};
    fragmentCreateInfo.setCode(fragmentSpirv);

    auto const vertexShader = shaderInfo.device.createShaderModuleUnique(vertexCreateInfo);
    auto const fragmentShader = shaderInfo.device.createShaderModuleUnique(fragmentCreateInfo);
    auto const pipelineState = PipelineState{
            .vertexShader = *vertexShader,
            .fragmentShader = *fragmentShader,
    };
    PipelineBuilderCreateInfo pipelineBuilderCreateInfo{
            .device = shaderInfo.device,
            .samples = shaderInfo.samples,
            .colorFormat = shaderInfo.colorFormat,
            .depthFormat = shaderInfo.depthFormat
    };
    PipelineBuilder pipelineBuilder(pipelineBuilderCreateInfo);
    m_Pipeline = pipelineBuilder.Build(shaderInfo.pipelineLayout, pipelineState);
}

void ShaderPipeline::Bind() const {
    m_APIRenderer->BindShader(*m_Pipeline);
    m_APIRenderer->BindDescriptorSets();
    m_APIRenderer->SetPolygonMode(m_IsWireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);
    //m_APIRenderer->SetAndUpdateMSAA(4);
    m_APIRenderer->SetLineWidth(m_LineWidth);
}
void ShaderPipeline::Unbind() const {

}

void ShaderPipeline::SetUniformInt(const std::string &name, int value) {

}

void ShaderPipeline::SetUniformMat4(const std::string &name, const glm::mat4 &matrix) {

}

void ShaderPipeline::SetUniformFloat(const std::string &name, float value) {

}

void ShaderPipeline::SetUniformVec4(const std::string &name, const glm::vec4 &value) {

}

int ShaderPipeline::GetUniformLocation(const std::string &name) {
    return 0;
}


