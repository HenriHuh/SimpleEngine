#pragma once

#include "Camera.h"
#include "Transform.h"

#include <vector>

struct GameInput
{
    float moveForward = 0.0f;
    float moveRight = 0.0f;
    float lookHorizontal = 0.0f;
    float lookVertical = 0.0f;
    bool shoot = false;
};

struct Enemy
{
    Transform transform;
    int health = 3;
};

struct Projectile
{
    Transform transform;
    glm::vec3 velocity{0.0f};
    float remainingLifetime = 3.0f;
    int damage = 1;
};

class Game
{
public:
    Game();

    void update(float deltaTime, const GameInput& input);

    const Camera& getCamera() const;
    const Enemy& getEnemy() const;
    const std::vector<Projectile>& getProjectiles() const;

private:
    void shoot();
    void updateProjectiles(float deltaTime);
    void spawnEnemy();

    Camera m_camera;
    Enemy m_enemy;
    std::vector<Projectile> m_projectiles;
    float m_shotCooldown = 0.0f;
};
