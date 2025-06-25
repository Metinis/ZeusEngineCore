#include "VKShader.h"
#include "../../Utils.h"
void VKShader::Init(const ShaderInfo& shaderInfo) {

    if (!std::holds_alternative<VulkanShaderInfo>(shaderInfo.backendData)) {
        throw std::runtime_error{ "Invalid Shader Info Data For Vulkan!" };
    }

    const auto& vkShaderInfo = std::get<VulkanShaderInfo>(shaderInfo.backendData);

    //initialise members
    m_VertexInput = vkShaderInfo.vertexInput;

    //load spirv
    const auto vertexSpirv = ToSpirV(shaderInfo.vertexPath);
    const auto fragmentSpirv = ToSpirV(shaderInfo.fragmentPath);

    auto const createShaderCreateInfo = [&vkShaderInfo](std::span<std::uint32_t const> spirv) {
        auto ret = vk::ShaderCreateInfoEXT{};
        ret.setCodeSize(spirv.size_bytes())
            .setPCode(spirv.data())
            .setSetLayouts(vkShaderInfo.setLayouts)
            .setCodeType(vk::ShaderCodeTypeEXT::eSpirv)
            .setPName("main");
        return ret;
        };

    auto shaderCreateInfos = std::array{
        createShaderCreateInfo(vertexSpirv),
        createShaderCreateInfo(fragmentSpirv),
    };

    shaderCreateInfos[0]
        .setStage(vk::ShaderStageFlagBits::eVertex)
        .setNextStage(vk::ShaderStageFlagBits::eFragment);
    shaderCreateInfos[1].setStage(vk::ShaderStageFlagBits::eFragment);

    auto result = vkShaderInfo.device.createShadersEXTUnique(shaderCreateInfos);
    if (result.result != vk::Result::eSuccess) {
        throw std::runtime_error{ "Failed to create Shader Objects" };
    }
    m_Shaders = std::move(result.value);
    m_Waiter = vkShaderInfo.device;
    
}
VKShader::~VKShader() = default;


void VKShader::Bind(vk::CommandBuffer commandBuffer, glm::ivec2 const framebufferSize) {
    SetViewportScissor(commandBuffer, framebufferSize);
    SetStaticStates(commandBuffer);
    SetCommonStates(commandBuffer);
    SetVertexStates(commandBuffer);
    SetFragmentStates(commandBuffer);
    BindShaders(commandBuffer);
}

void VKShader::Unbind() const {

}

void VKShader::SetUniformInt(const std::string &name, int value) {

}
void VKShader::SetUniformMat4(const std::string &name, const glm::mat4 &matrix) {

}
void VKShader::SetUniformFloat(const std::string &name, float value) {

}
void VKShader::SetUniformVec4(const std::string &name, const glm::vec4 &value) {

}
int VKShader::GetUniformLocation(const std::string &name) {
	return 0;
}
void VKShader::Bind() const {
    throw std::runtime_error("Use Bind(commandBuffer) for Vulkan shaders");
}
constexpr auto toVKbool(bool const value) {
    return value ? vk::True : vk::False;
}
void VKShader::SetViewportScissor(vk::CommandBuffer const commandBuffer, glm::ivec2 const framebufferSize) {
    auto const fsize = glm::vec2{ framebufferSize };
    vk::Viewport viewport{};
    viewport.setX(0.0f).setY(fsize.y).setWidth(fsize.x).setHeight(-fsize.y);
    commandBuffer.setViewportWithCount(viewport);

    auto const usize = glm::uvec2{ framebufferSize };
    auto const scissor = vk::Rect2D{ vk::Offset2D{}, vk::Extent2D{usize.x, usize.y} };
    commandBuffer.setScissorWithCount(scissor);
}

void VKShader::SetStaticStates(vk::CommandBuffer const commandBuffer) {
    commandBuffer.setRasterizerDiscardEnable(vk::False);
    commandBuffer.setRasterizationSamplesEXT(vk::SampleCountFlagBits::e1);
    commandBuffer.setSampleMaskEXT(vk::SampleCountFlagBits::e1, 0xff);
    commandBuffer.setAlphaToCoverageEnableEXT(vk::False);
    commandBuffer.setCullMode(vk::CullModeFlagBits::eNone);
    commandBuffer.setFrontFace(vk::FrontFace::eCounterClockwise);
    commandBuffer.setDepthBiasEnable(vk::False);
    commandBuffer.setStencilTestEnable(vk::False);
    commandBuffer.setPrimitiveRestartEnable(vk::False);
    commandBuffer.setColorWriteMaskEXT(0, ~vk::ColorComponentFlags{});
}

void VKShader::SetCommonStates(vk::CommandBuffer const commandBuffer) const {
    auto const depth_test = toVKbool((flags & DepthTest) == DepthTest);
    commandBuffer.setDepthWriteEnable(depth_test);
    commandBuffer.setDepthTestEnable(depth_test);
    commandBuffer.setDepthCompareOp(depthCompare_op);
    commandBuffer.setPolygonModeEXT(polygonMode);
    commandBuffer.setLineWidth(line_width);
}

void VKShader::SetVertexStates(
    vk::CommandBuffer const commandBuffer) const {
    commandBuffer.setVertexInputEXT(m_VertexInput.bindings,
        m_VertexInput.attributes);
    commandBuffer.setPrimitiveTopology(topology);
}

void VKShader::SetFragmentStates(
    vk::CommandBuffer const commandBuffer) const {
    auto const alpha_blend = toVKbool((flags & AlphaBlend) == AlphaBlend);
    commandBuffer.setColorBlendEnableEXT(0, alpha_blend);
    commandBuffer.setColorBlendEquationEXT(0, colorBlendEquation);
}

void VKShader::BindShaders(vk::CommandBuffer const commandBuffer) const {
    static constexpr auto stages_v = std::array{
      vk::ShaderStageFlagBits::eVertex,
      vk::ShaderStageFlagBits::eFragment,
    };
    auto const shaders = std::array{
      *m_Shaders[0],
      *m_Shaders[1],
    };
    commandBuffer.bindShadersEXT(stages_v, shaders);
}




