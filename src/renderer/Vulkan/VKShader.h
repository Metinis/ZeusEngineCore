
#pragma once
#include "ZeusEngineCore/IShader.h"
#include "ZeusEngineCore/ScopedWaiter.h"

class VKShader : public IShader{
public:
    ~VKShader() override;
    void Init(const ShaderInfo& shaderInfo) override;

    void Bind() const override;
    void Bind(vk::CommandBuffer commandBuffer, glm::ivec2 const framebufferSize) override;
    void Unbind() const override;

    void SetUniformInt(const std::string& name, int value) override;
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix) override;
    void SetUniformFloat(const std::string& name, float value) override;
    void SetUniformVec4(const std::string& name, const glm::vec4& value) override;

    static constexpr auto colorBlendEquation_v = [] {
        auto ret = vk::ColorBlendEquationEXT{};
        ret.setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha);
        return ret;
        }();

    vk::PrimitiveTopology topology{ vk::PrimitiveTopology::eTriangleList };
    vk::PolygonMode polygonMode{ vk::PolygonMode::eFill };
    float line_width{ 1.0f };
    vk::ColorBlendEquationEXT colorBlendEquation{ colorBlendEquation_v };
    vk::CompareOp depthCompare_op{ vk::CompareOp::eLessOrEqual };

    enum : std::uint8_t {
        None = 0,
        AlphaBlend = 1 << 0,
        DepthTest = 1 << 1,
    };

    static constexpr auto flags_v = AlphaBlend | DepthTest;

    std::uint8_t flags{ flags_v };

private:
    std::vector<vk::UniqueShaderEXT> m_Shaders{};
    VKShaderVertexInput m_VertexInput{};
    ScopedWaiter m_Waiter{};

    int GetUniformLocation(const std::string& name) override;
    static void SetViewportScissor(vk::CommandBuffer command_buffer, glm::ivec2 framebuffer);
    static void SetStaticStates(vk::CommandBuffer command_buffer);
    void SetCommonStates(vk::CommandBuffer command_buffer) const;
    void SetVertexStates(vk::CommandBuffer command_buffer) const;
    void SetFragmentStates(vk::CommandBuffer command_buffer) const;
    void BindShaders(vk::CommandBuffer command_buffer) const;
};
