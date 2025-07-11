#pragma once

#include "ZeusEngineCore/IShader.h"
#include "Backend/VulkanPipelineBuilder.h"
#include "ZeusEngineCore/ScopedWaiter.h"

class VKShaderPipeline : public IShader{
public:
    ~VKShaderPipeline() override = default;
    void Init(const ShaderInfo& shaderInfo) override;

    void Bind() const override;
    void Bind(vk::CommandBuffer commandBuffer, vk::Extent2D extent) override;
    void Unbind() const override;

    void SetUniformInt(const std::string& name, int value) override;
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix) override;
    void SetUniformFloat(const std::string& name, float value) override;
    void SetUniformVec4(const std::string& name, const glm::vec4& value) override;
    void ToggleWireframe() override;

private:
    vk::UniquePipeline m_Pipeline;
    ScopedWaiter m_Waiter;
    int GetUniformLocation(const std::string& name) override;
};
