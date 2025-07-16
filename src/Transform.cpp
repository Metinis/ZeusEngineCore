#include "ZeusEngineCore/Transform.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

using namespace ZEN;
struct Matrices {
    glm::mat4 translation;
    glm::mat4 orientation;
    glm::mat4 scale;
};
[[nodiscard]] auto toMatrices(glm::vec2 const position, float rotation,
                               glm::vec2 const scale) -> Matrices {
    static constexpr auto mat_v = glm::identity<glm::mat4>();
    static constexpr auto axis_v = glm::vec3{0.0f, 0.0f, 1.0f};
    return Matrices{
            .translation = glm::translate(mat_v, glm::vec3{position, 0.0f}),
            .orientation = glm::rotate(mat_v, glm::radians(rotation), axis_v),
            .scale = glm::scale(mat_v, glm::vec3{scale, 1.0f}),
    };
}

glm::mat4 Transform::modelMatrix() const {
    auto const [t, r, s] = toMatrices(position, rotation, scale);
    return t * r * s;
}

glm::mat4 Transform::viewMatrix() const {
    auto const [t, r, s] = toMatrices(-position, -rotation, scale);
    return r * t * s;
}
