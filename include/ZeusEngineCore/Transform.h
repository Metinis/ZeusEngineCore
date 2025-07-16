#pragma once
#include <glm/glm.hpp>

namespace ZEN {
    struct Transform {
        glm::vec2 position{};
        float rotation = 0.0f;
        glm::vec2 scale{1.0f, 1.0f};

        [[nodiscard]] glm::mat4 modelMatrix() const;

        [[nodiscard]] glm::mat4 viewMatrix() const;
    };
}