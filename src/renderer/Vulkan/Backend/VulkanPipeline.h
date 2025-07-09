#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "ZeusEngineCore/Vertex.h"
struct PipelineFlag {
    enum : std::uint8_t {
        None = 0,
        AlphaBlend = 1 << 0,
        DepthTest = 1 << 1,
    };
};
struct PipelineState {

    [[nodiscard]] static constexpr auto defaultFlags() -> std::uint8_t {
        return PipelineFlag::AlphaBlend | PipelineFlag::DepthTest;
    }

    vk::ShaderModule vertexShader;
    vk::ShaderModule fragmentShader;

    std::span<vk::VertexInputAttributeDescription const> vertexAttributes{vertexAttributes_v};
    std::span<vk::VertexInputBindingDescription const> vertexBindings{vertexBindings_v};

    vk::PrimitiveTopology topology{vk::PrimitiveTopology::eTriangleList};
    vk::PolygonMode polygonMode{vk::PolygonMode::eFill};
    vk::CullModeFlags cullMode{vk::CullModeFlagBits::eNone};
    vk::CompareOp depthCompare{vk::CompareOp::eLess};
    std::uint8_t flags{defaultFlags()};
};
struct PipelineCreateInfo {
    vk::Device device{};
    vk::SampleCountFlagBits samples{};
    vk::Format colorFormat{};
    vk::Format depthFormat{};
    vk::PipelineLayout pipelineLayout{};
};
class Pipeline{
public:
    Pipeline(PipelineCreateInfo const& pipelineCreateInfo);
    void Build(PipelineState const& state);
    const vk::Pipeline Get() const {return *m_Pipeline;}
private:
    PipelineCreateInfo m_CreateInfo{};
    vk::UniquePipeline m_Pipeline{};
    vk::PipelineLayout m_PipelineLayout{};
};