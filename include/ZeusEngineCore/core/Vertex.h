#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

namespace ZEN {
    struct Vertex {
        glm::vec3 position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec4 Color;
        glm::vec3 Tangent;
        glm::vec3 Bitangent;
    };

}
