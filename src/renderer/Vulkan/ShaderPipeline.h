#pragma once

#include "ZeusEngineCore/IShader.h"
#include "Backend/PipelineBuilder.h"
#include "ZeusEngineCore/ScopedWaiter.h"

namespace ZEN::VKAPI {
    class APIRenderer;

    struct ShaderInfo {
        std::string vertexPath;
        std::string fragmentPath;
        vk::Device device{};
        vk::SampleCountFlagBits samples{};
        vk::Format colorFormat{};
        vk::Format depthFormat{};
        vk::PipelineLayout pipelineLayout{};
        APIRenderer* apiRenderer;
    };
class ShaderPipeline : public ZEN::IShader {
    public:
        explicit ShaderPipeline(const ZEN::VKAPI::ShaderInfo &shaderInfo);

        ~ShaderPipeline() override = default;

        void Bind() const override;

        void Unbind() const override;

        void SetUniformInt(const std::string &name, int value) override;

        void SetUniformMat4(const std::string &name, const glm::mat4 &matrix) override;

        void SetUniformFloat(const std::string &name, float value) override;

        void SetUniformVec4(const std::string &name, const glm::vec4 &value) override;

    private:
        vk::UniquePipeline m_Pipeline;
        ScopedWaiter m_Waiter;
        APIRenderer* m_APIRenderer;

        int GetUniformLocation(const std::string &name) override;
    };
}
