#pragma once
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/constants.hpp>

namespace ZEN {
    struct Matrices {
        glm::mat4 translation;
        glm::mat4 orientation;
        glm::mat4 scale;
    };
    Matrices toMatrices(glm::vec3 const position, glm::vec3 rotation,
                                  glm::vec3 const scale);
    std::string fullPath(const std::string& path);

}


