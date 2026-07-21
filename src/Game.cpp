#include "Game.h"

void Game::update(float deltaTime, const GameInput& input)
{
    m_camera.look(input.lookHorizontal, input.lookVertical);
    m_camera.move(input.moveForward, input.moveRight, deltaTime);

    // The cube remains a rotating target placeholder until enemy ships are
    // introduced.
    m_targetTransform.rotation.x += 0.4f * deltaTime;
    m_targetTransform.rotation.y += 1.0f * deltaTime;
}

const Camera& Game::getCamera() const
{
    return m_camera;
}

const Transform& Game::getTargetTransform() const
{
    return m_targetTransform;
}
