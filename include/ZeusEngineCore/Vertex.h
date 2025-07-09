#pragma once
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/glm.hpp>
struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec4 Color;
};
constexpr auto vertexAttributes2EXT_v = std::array{
        vk::VertexInputAttributeDescription2EXT{0, 0, vk::Format::eR32G32B32Sfloat,
                                                offsetof(Vertex, Position)},
        vk::VertexInputAttributeDescription2EXT{1, 0, vk::Format::eR32G32B32Sfloat,
                                                offsetof(Vertex, Normal)},
        vk::VertexInputAttributeDescription2EXT{2, 0, vk::Format::eR32G32Sfloat,
                                                offsetof(Vertex, TexCoords)},
        vk::VertexInputAttributeDescription2EXT{3, 0, vk::Format::eR32G32B32A32Sfloat,
                                                offsetof(Vertex, Color)},
};

constexpr auto vertexBindings2EXT_v = std::array{
        vk::VertexInputBindingDescription2EXT{0, sizeof(Vertex),
                                              vk::VertexInputRate::eVertex, 1},
};
constexpr auto vertexAttributes_v = std::to_array({
                                                          vk::VertexInputAttributeDescription{0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Position)},
                                                          vk::VertexInputAttributeDescription{1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal)},
                                                          vk::VertexInputAttributeDescription{2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoords)},
                                                          vk::VertexInputAttributeDescription{3, 0, vk::Format::eR32G32B32A32Sfloat, offsetof(Vertex, Color)},
                                                  });

constexpr auto vertexBindings_v = std::to_array({
                                                        vk::VertexInputBindingDescription{0, sizeof(Vertex), vk::VertexInputRate::eVertex},
                                                });
