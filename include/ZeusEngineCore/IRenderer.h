#pragma once
#include <memory>
#include "ZeusEngineCore/IMesh.h"
#include "ZeusEngineCore/Material.h"
#include "optional"
#include <vulkan/vulkan.hpp>
#include "../src/Utils.h"
#include <variant>
#include <ZeusEngineCore/InfoVariants.h>
#include <functional>

namespace ZEN {
    enum class RendererAPI;

    using RendererContextVariant = std::variant<std::monostate, VKAPI::ContextInfo, OGLAPI::ContextInfo>;
    using ShaderInfoVariant = std::variant<std::monostate, VKAPI::ShaderInfo, OGLAPI::ShaderInfo>;

    struct RenderCommand {
        glm::mat4 transform;
        std::shared_ptr<Material> material;
        std::shared_ptr<IMesh> mesh;
    };

    class IRenderer {
    public:
        virtual ~IRenderer() = default;

        virtual void Init(RendererInitInfo &initInfo) = 0;

        virtual bool BeginFrame() = 0;

        virtual void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                            const std::shared_ptr<IMesh> &mesh) = 0;

        virtual void EndFrame(const std::function<void(vk::CommandBuffer)> &uiExtraDrawCallback = nullptr) = 0;

        virtual void DrawMesh(const IMesh &mesh, Material &material) = 0;

        static std::unique_ptr<IRenderer> Create(RendererAPI api);

        // For Vulkan: this will be used for ImGui EndFrame command buffer parameter
        virtual void *GetCurrentCommandBuffer() = 0;

        virtual RendererContextVariant GetContext() const = 0;

        virtual ShaderInfoVariant GetShaderInfo() const = 0;

    protected:
        std::vector<RenderCommand> m_RenderQueue;
    };
}
