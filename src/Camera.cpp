#include "Camera.h"

#include <glm/gtc/matrix_transform.hpp>

glm::mat4 Camera::getViewMatrix() const
{
    return glm::lookAt(m_position, m_position + m_forward, m_up);
}

void Camera::move(float forwardInput, float rightInput, float deltaTime)
{
    const glm::vec3 right = glm::normalize(glm::cross(m_forward, m_up));
    glm::vec3 movement = m_forward * forwardInput + right * rightInput;

    // Normalizing prevents diagonal movement from being faster than movement
    // along a single axis.
    if (glm::dot(movement, movement) > 0.0f)
    {
        movement = glm::normalize(movement);
        m_position += movement * m_movementSpeed * deltaTime;
    }
}
