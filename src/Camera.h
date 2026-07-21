#pragma once

#include "Transform.h"

class Camera
{
public:
    glm::mat4 getViewMatrix() const;
    const Transform& getTransform() const;
    glm::vec3 getForwardDirection() const;

    void move(float forwardInput, float rightInput, float deltaTime);
    void look(float horizontalOffset, float verticalOffset);

private:
    glm::vec3 getRightDirection() const;

    Transform m_transform{
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f),
        glm::vec3(1.0f)};
    float m_movementSpeed = 2.5f;
    float m_mouseSensitivityDegrees = 0.1f;
};
