
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
        void Init(RendererInitInfo &initInfo);

        ~Renderer();

        bool BeginFrame();

        void Submit(const glm::mat4 &transform, const std::shared_ptr<Material> &material,
                    const std::shared_ptr<IMesh> &mesh);

        void EndFrame(const std::function<void(void*)>& uiExtraDrawCallback = nullptr);


        BackendContextVariant GetContext() const;

        VKAPI::APIRenderer* GetAPIRenderer() const;

        VKAPI::APIBackend* GetAPIBackend() const;

    private:
        void UpdateView();

        std::vector<RenderCommand> m_RenderQueue;
        //VKAPI placeholder until interface
        std::unique_ptr<VKAPI::APIBackend> m_Backend;
        std::unique_ptr<VKAPI::APIRenderer> m_APIRenderer;
        std::optional<VKAPI::DescriptorBuffer> m_ViewUBO{};
        VKAPI::Texture m_Texture;
    };
}
