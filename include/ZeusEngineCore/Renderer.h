#pragma once
#include <optional>
#include <memory>
#include <glm/glm.hpp>
#include <functional>

namespace ZEN {
    struct RendererInitInfo;
    class IRendererAPI;
    class IRendererBackend;
    class IDescriptorBuffer;
    class Material;
    class IMesh;
    struct RenderCommand {
        glm::mat4 transform;
        std::shared_ptr<Material> material;
        std::shared_ptr<IMesh> mesh;
    };
    class Renderer{
    public:
        explicit Renderer(RendererInitInfo &initInfo);

        ~Renderer();

        bool BeginFrame();

        void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                    const std::shared_ptr<IMesh> &mesh);

        void EndFrame(const std::function<void(void*)>& uiExtraDrawCallback = nullptr);

        [[nodiscard]] IRendererAPI* GetAPIRenderer() const;

        [[nodiscard]] IRendererBackend* GetAPIBackend() const;

    private:
        void UpdateView();

        std::vector<RenderCommand> m_RenderQueue;
        std::unique_ptr<IRendererBackend> m_Backend;
        std::unique_ptr<IRendererAPI> m_APIRenderer;
        std::unique_ptr<IDescriptorBuffer> m_ViewUBO;
    };
}
