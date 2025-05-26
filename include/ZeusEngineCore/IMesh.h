#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
#include <vector>
#include "RendererAPI.h"
#include "Material.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec4 Color;
};

class IMesh {
public:
    virtual ~IMesh() = default;
    virtual void Init(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices) = 0;
    virtual void Draw(Material& material) const = 0;
    virtual void Cleanup() const = 0;

    static std::shared_ptr<IMesh> Create(RendererAPI api);
};