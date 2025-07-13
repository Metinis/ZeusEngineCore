#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>

namespace ZEN {
    struct Vertex {
        glm::vec3 Position;
        glm::vec3 Normal;
        glm::vec2 TexCoords;
        glm::vec4 Color;
    };
    constexpr auto vertexAttributes_v = std::to_array({
                                                              vk::VertexInputAttributeDescription{0, 0,
                                                                                                  vk::Format::eR32G32B32Sfloat,
                                                                                                  offsetof(Vertex,
                                                                                                           Position)},
                                                              vk::VertexInputAttributeDescription{1, 0,
                                                                                                  vk::Format::eR32G32B32Sfloat,
                                                                                                  offsetof(Vertex,
                                                                                                           Normal)},
                                                              vk::VertexInputAttributeDescription{2, 0,
                                                                                                  vk::Format::eR32G32Sfloat,
                                                                                                  offsetof(Vertex,
                                                                                                           TexCoords)},
                                                              vk::VertexInputAttributeDescription{3, 0,
                                                                                                  vk::Format::eR32G32B32A32Sfloat,
                                                                                                  offsetof(Vertex,
                                                                                                           Color)},
                                                      });

    constexpr auto vertexBindings_v = std::to_array({
                                                            vk::VertexInputBindingDescription{0, sizeof(Vertex),
                                                                                              vk::VertexInputRate::eVertex},
                                                    });

}
