#pragma once
#include <optional>
#include <memory>
#include <glm/glm.hpp>
#include <functional>
#include "Transform.h"
#include "Utils.h"

namespace ZEN {
    struct RendererInitInfo;
    class IRendererAPI;
    class IRendererBackend;
    class IDescriptorBuffer;
    class Material;
    class IMesh;
    struct RenderCommand {
        std::vector<Transform> transforms; //num of instances
        std::shared_ptr<IMesh> mesh;
        std::shared_ptr<Material> material;
    };
    class Renderer{
    public:
        explicit Renderer(RendererInitInfo &initInfo);

        ~Renderer();

        bool BeginFrame();

        void Submit(const std::vector<Transform>& transforms, const std::shared_ptr<IMesh> &mesh,
                    const std::shared_ptr<Material>& material);

        void EndFrame(const std::function<void(void*)>& uiExtraDrawCallback = nullptr);

        void UpdateInstances(const std::vector<Transform>& instances);

        [[nodiscard]] Transform& ViewMatrix(){return m_ViewTransform;}

        [[nodiscard]] IRendererAPI* GetAPIRenderer() const;

        [[nodiscard]] IRendererBackend* GetAPIBackend() const;

        std::uint32_t GetMSAA() const { return m_MSAA; }
        void SetMSAA(std::uint32_t msaa) { m_MSAA = msaa; }
    private:
        void UpdateView();

        WindowHandle m_WindowHandle{};
        std::vector<RenderCommand> m_RenderQueue;
        std::unique_ptr<IRendererBackend> m_Backend;
        std::unique_ptr<IRendererAPI> m_APIRenderer;
        std::unique_ptr<IDescriptorBuffer> m_ViewUBO;
        std::unique_ptr<IDescriptorBuffer> m_InstanceSSBO;
        Transform m_ViewTransform;
        std::uint32_t m_MSAA;
    };
}
