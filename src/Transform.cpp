#include "Transform.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Transform::getMatrix() const
{
    glm::mat4 model = glm::translate(glm::mat4(1.0f), position);

    // Rotation values are Euler angles in radians. The matrix order causes X
    // (pitch) to be applied first, followed by Y (yaw), then Z (roll).
    model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

    return glm::scale(model, scale);
}
