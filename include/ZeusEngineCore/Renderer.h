
#pragma once
#include "../../src/renderer/Vulkan/Backend/APIBackend.h"
#include "../../src/renderer/Vulkan/Backend/DescriptorBuffer.h"
#include "../../src/renderer/Vulkan/Texture.h"
#include "../../src/renderer/Vulkan/Backend/APIRenderer.h"
#include <optional>
#include "InfoVariants.h"

namespace ZEN {
    class Material;
    class IMesh;
    struct RenderCommand {
        glm::mat4 transform;
        std::shared_ptr<Material> material;
        std::shared_ptr<IMesh> mesh;
    };
    class Renderer{
    public:
        Renderer(RendererInitInfo &initInfo);

        ~Renderer();

        bool BeginFrame();

        void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                    const std::shared_ptr<IMesh> &mesh);

        void EndFrame(const std::function<void(void*)>& uiExtraDrawCallback = nullptr);

        IRendererAPI* GetAPIRenderer() const;

        IRendererBackend* GetAPIBackend() const;

    private:
        void UpdateView();

        std::vector<RenderCommand> m_RenderQueue;
        //VKAPI placeholder until interface
        std::unique_ptr<IRendererBackend> m_Backend;
        std::unique_ptr<IRendererAPI> m_APIRenderer;
        std::unique_ptr<IDescriptorBuffer> m_ViewUBO;
    };
}
