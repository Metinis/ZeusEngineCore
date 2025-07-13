#pragma once
#include <cstdint>
#include <vulkan/vulkan.hpp>
#include "ZeusEngineCore/Vertex.h"

namespace ZEN::VKAPI {
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

    struct PipelineBuilderCreateInfo {
        vk::Device device{};
        vk::SampleCountFlagBits samples{};
        vk::Format colorFormat{};
        vk::Format depthFormat{};
    };

    class PipelineBuilder {
    public:
        explicit PipelineBuilder(PipelineBuilderCreateInfo const &pipelineBuilderCreateInfo);

        vk::UniquePipeline Build(vk::PipelineLayout layout, PipelineState const &state);

    private:
        PipelineBuilderCreateInfo m_CreateInfo{};
    };
}