#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <vector>
#include "RendererAPI.h"
#include "Material.h"
#include "ZeusEngineCore/Vertex.h"
#include "InfoVariants.h"

namespace ZEN{
    class IMesh {
    public:
        virtual ~IMesh() = default;
        virtual void Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices,
                          const BackendContextVariant& context) = 0;
        virtual void Draw(Material& material) const = 0;

        virtual void Draw(Material& material, vk::CommandBuffer commandBuffer) {
            // By default dont support command buffer
            throw std::runtime_error("Bind with command buffer not implemented for this mesh type");
        }

        static std::shared_ptr<IMesh> Create(RendererAPI api, VKAPI::APIRenderer* apiRenderer);
    };
}
