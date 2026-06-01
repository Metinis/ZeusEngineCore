#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

namespace ZEN {
    struct Vertex {
        glm::vec3 position;
        float _pad0;
        glm::vec3 Normal;
        float _pad1;
        glm::vec2 TexCoords;
        float _pad2;
        float _pad3;
        glm::vec4 Color;
        glm::vec3 Tangent;
        float _pad4;
        glm::vec3 Bitangent;
        float _pad5;
    };

}
