#include "Camera.h"

namespace
{
constexpr glm::vec3 WorldUp{0.0f, 1.0f, 0.0f};
}

glm::mat4 Camera::getViewMatrix() const
{
    // A view matrix moves the world in the opposite direction of the camera,
    // so it is the inverse of the camera's world transform.
    return glm::inverse(m_transform.getMatrix());
}

const Transform& Camera::getTransform() const
{
    return m_transform;
}

void Camera::move(float forwardInput, float rightInput, float deltaTime)
{
    // Ignore the vertical part of the look direction so WASD movement stays
    // parallel to the ground while the player looks up or down.
    const glm::vec3 forward = getForwardDirection();
    const glm::vec3 groundForward = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));
    const glm::vec3 right = glm::normalize(glm::cross(groundForward, WorldUp));
    glm::vec3 movement = groundForward * forwardInput + right * rightInput;

    // Normalizing prevents diagonal movement from being faster than movement
    // along a single axis.
    if (glm::dot(movement, movement) > 0.0f)
    {
        movement = glm::normalize(movement);
        m_transform.position += movement * m_movementSpeed * deltaTime;
    }
}

void Camera::look(float horizontalOffset, float verticalOffset)
{
    const float sensitivityRadians = glm::radians(m_mouseSensitivityDegrees);
    m_transform.rotation.y -= horizontalOffset * sensitivityRadians;
    m_transform.rotation.x += verticalOffset * sensitivityRadians;

    // Looking exactly straight up or down makes the forward and up directions
    // parallel, so stop just short of 90 degrees to keep the view stable.
    const float pitchLimit = glm::radians(89.0f);
    m_transform.rotation.x = glm::clamp(m_transform.rotation.x, -pitchLimit, pitchLimit);
}

glm::vec3 Camera::getForwardDirection() const
{
    const glm::vec4 localForward{0.0f, 0.0f, -1.0f, 0.0f};
    return glm::normalize(glm::vec3(m_transform.getMatrix() * localForward));
}
