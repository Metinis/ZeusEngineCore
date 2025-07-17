#pragma once
#include <memory>
#include <vector>
#include <cstdint>

namespace ZEN{
    struct Vertex;
    class IRendererBackend;
    class IRendererAPI;
    namespace OGLAPI{
        class APIBackend;
    }
    class IMesh {
    public:
        virtual ~IMesh() = default;
        virtual void Draw(std::uint32_t instanceCount) const = 0;

        static std::shared_ptr<IMesh> Create(IRendererBackend* apiBackend,
                                             IRendererAPI* apiRenderer,
                                             const std::vector<Vertex>& vertices,
                                             const std::vector<uint32_t>& indices);
    };

}
