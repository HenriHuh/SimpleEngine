#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
    glm::mat4 getViewMatrix() const;

    void move(float forwardInput, float rightInput, float deltaTime);

private:
    glm::vec3 m_position{0.0f, 0.0f, 3.0f};
    glm::vec3 m_forward{0.0f, 0.0f, -1.0f};
    glm::vec3 m_up{0.0f, 1.0f, 0.0f};
    float m_movementSpeed = 2.5f;
};
