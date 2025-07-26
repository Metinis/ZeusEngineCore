#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace ZEN {
    struct Transform {
        glm::vec3 position{};
        glm::vec3 rotation{0.0f, 0.0f, 0.0f};
        glm::vec3 scale{1.0f, 1.0f, 1.0f};

        [[nodiscard]] glm::mat4 modelMatrix() const;

        [[nodiscard]] glm::mat4 viewMatrix() const;
    };
}