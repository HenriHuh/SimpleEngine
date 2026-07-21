#pragma once

#include "Camera.h"
#include "Transform.h"

struct GameInput
{
    float moveForward = 0.0f;
    float moveRight = 0.0f;
    float lookHorizontal = 0.0f;
    float lookVertical = 0.0f;
};

class Game
{
public:
    void update(float deltaTime, const GameInput& input);

    const Camera& getCamera() const;
    const Transform& getTargetTransform() const;

private:
    Camera m_camera;
    Transform m_targetTransform;
};
