#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <vector>
#include "IRendererAPI.h"
#include "Material.h"
#include "ZeusEngineCore/Vertex.h"
#include "InfoVariants.h"

namespace ZEN{
    namespace OGLAPI{
        class APIBackend;
    }
    class IMesh {
    public:
        virtual ~IMesh() = default;
        virtual void Draw() const = 0;

        static std::shared_ptr<IMesh> Create(IRendererBackend* apiBackend,
                                             IRendererAPI* apiRenderer,
                                             const std::vector<Vertex>& vertices,
                                             const std::vector<uint32_t>& indices);
    };
}
