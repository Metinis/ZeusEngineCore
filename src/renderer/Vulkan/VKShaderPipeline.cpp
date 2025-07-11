#include "VKShaderPipeline.h"
#include "../../Utils.h"

void VKShaderPipeline::Init(const ShaderInfo& shaderInfo) {
    const auto vertexSpirv = ToSpirV(shaderInfo.vertexPath);
    const auto fragmentSpirv = ToSpirV(shaderInfo.fragmentPath);

    if (vertexSpirv.empty() || fragmentSpirv.empty()) {
        throw std::runtime_error{"Failed to load shaders"};
    }
    if (!std::holds_alternative<VulkanShaderInfo>(shaderInfo.backendData)) {
        throw std::runtime_error{ "Invalid Shader Info Data For Vulkan!" };
    }

    const auto& vkShaderInfo = std::get<VulkanShaderInfo>(shaderInfo.backendData);

    m_Waiter = vkShaderInfo.device;

    vk::ShaderModuleCreateInfo vertexCreateInfo{};
    vertexCreateInfo.setCode(vertexSpirv);

    vk::ShaderModuleCreateInfo fragmentCreateInfo{};
    fragmentCreateInfo.setCode(fragmentSpirv);

    auto const vertexShader = vkShaderInfo.device.createShaderModuleUnique(vertexCreateInfo);
    auto const fragmentShader = vkShaderInfo.device.createShaderModuleUnique(fragmentCreateInfo);
    auto const pipelineState = PipelineState{
            .vertexShader = *vertexShader,
            .fragmentShader = *fragmentShader,
    };
    PipelineBuilderCreateInfo pipelineBuilderCreateInfo{
        .device = vkShaderInfo.device,
        .samples = vkShaderInfo.samples,
        .colorFormat = vkShaderInfo.colorFormat,
        .depthFormat = vkShaderInfo.depthFormat
    };
    PipelineBuilder pipelineBuilder(pipelineBuilderCreateInfo);
    m_Pipeline = pipelineBuilder.Build(vkShaderInfo.pipelineLayout, pipelineState);
}

void VKShaderPipeline::Bind() const {

}

void VKShaderPipeline::Bind(vk::CommandBuffer commandBuffer, const vk::Extent2D extent) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *m_Pipeline);
    commandBuffer.setPolygonModeEXT(m_IsWireframe ? vk::PolygonMode::eLine : vk::PolygonMode::eFill);
    commandBuffer.setLineWidth(m_LineWidth);
    vk::Viewport viewport{};
    viewport.setX(0.0f)
            .setY(static_cast<float>(extent.height))
            .setWidth(static_cast<float>(extent.width))
            .setHeight(-viewport.y);
    commandBuffer.setViewport(0, viewport);
    commandBuffer.setScissor(0, vk::Rect2D{{}, extent});
}

void VKShaderPipeline::Unbind() const {

}

void VKShaderPipeline::SetUniformInt(const std::string &name, int value) {

}

void VKShaderPipeline::SetUniformMat4(const std::string &name, const glm::mat4 &matrix) {

}

void VKShaderPipeline::SetUniformFloat(const std::string &name, float value) {

}

void VKShaderPipeline::SetUniformVec4(const std::string &name, const glm::vec4 &value) {

}

void VKShaderPipeline::ToggleWireframe() {

}

int VKShaderPipeline::GetUniformLocation(const std::string &name) {
    return 0;
}
