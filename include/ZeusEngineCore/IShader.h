#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <glm/mat4x4.hpp>
#include <variant>
#include "RendererAPI.h"
#include <string_view>
#include <memory>
#include <ZeusEngineCore/InfoVariants.h>
#include "../../src/renderer/Vulkan/Backend/APIRenderer.h"

namespace ZEN {
    struct ShaderInfo {
        RendererAPI api;
        std::string vertexPath;
        std::string fragmentPath;

        std::variant<std::monostate, VKAPI::ShaderInfo, OGLAPI::ShaderInfo> backendData;
    };

    class IShader {
    public:
        virtual void Init(const ShaderInfo &shaderInfo) = 0;

        virtual ~IShader() = default;

        virtual void Bind() const = 0;

        virtual void Bind(vk::CommandBuffer commandBuffer, const vk::Extent2D extent) {
            // By default dont support command buffer
            throw std::runtime_error("Bind with command buffer not implemented for this shader type");
        }

        virtual void Unbind() const = 0;

        virtual void SetUniformInt(const std::string &name, int value) = 0;

        virtual void SetUniformMat4(const std::string &name, const glm::mat4 &matrix) = 0;

        virtual void SetUniformFloat(const std::string &name, float value) = 0;

        virtual void SetUniformVec4(const std::string &name, const glm::vec4 &value) = 0;

        virtual void ToggleWireframe() = 0;

        bool *GetWireframeFlag() { return &m_IsWireframe; }

        float *GetLineWidth() { return &m_LineWidth; }

        static std::shared_ptr<IShader> Create(RendererAPI api, VKAPI::APIRenderer* apiRenderer);

    protected:
        std::uint32_t m_RendererID;
        bool m_IsWireframe = false;
        std::unordered_map<std::string, int> m_UniformLocationCache;
        float m_LineWidth{1.0f};

        virtual int GetUniformLocation(const std::string &name) = 0;
    };
}
