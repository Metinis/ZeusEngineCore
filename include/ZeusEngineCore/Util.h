#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace ZEN {
    struct Matrices {
        glm::mat4 translation;
        glm::mat4 orientation;
        glm::mat4 scale;
    };
    static auto toMatrices(glm::vec3 const position, glm::vec3 rotation,
                                  glm::vec3 const scale) -> Matrices {
        static constexpr auto mat_v = glm::identity<glm::mat4>();
        static constexpr auto axis_z = glm::vec3{0.0f, 0.0f, 1.0f};
        static constexpr auto axis_x = glm::vec3{1.0f, 0.0f, 0.0f};
        static constexpr auto axis_y = glm::vec3{0.0f, 1.0f, 0.0f};

        rotation = glm::radians(rotation);

        // Apply Z -> X -> Y rotation order
        glm::mat4 orientation = glm::rotate(mat_v, rotation.z, axis_z);
        orientation = glm::rotate(orientation, rotation.x, axis_x);
        orientation = glm::rotate(orientation, rotation.y, axis_y);

        return Matrices{
            .translation = glm::translate(mat_v, position),
            .orientation = orientation,
            .scale = glm::scale(mat_v, scale),
        };
    }
}


