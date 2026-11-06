#pragma once

#include "Camera.h"
#include "Transform.h"

#include <vector>

enum class EncounterState
{
    Playing,
    Victory,
    Defeat
};

struct GameInput
{
    float moveForward = 0.0f;
    float moveRight = 0.0f;
    float moveUp = 0.0f;
    float lookHorizontal = 0.0f;
    float lookVertical = 0.0f;
    bool fireRaycast = false;
    bool fireProjectile = false;
    bool restart = false;
};

struct Enemy
{
    static constexpr float CollisionRadius = 0.8f;

    Transform transform;
    int id = 0;
    int health = 15;

    float getCollisionRadius() const
    {
        const glm::vec3 scale = glm::abs(transform.scale);
        return CollisionRadius * glm::max(scale.x, glm::max(scale.y, scale.z));
    }
};

struct Projectile
{
    Transform transform;
    glm::vec3 velocity{0.0f};
    float remainingLifetime = 3.0f;
    int damage = 1;
};

struct RaycastTracer
{
    glm::vec3 start{0.0f};
    glm::vec3 end{0.0f};
    float lifetime = 0.15f;
    float remainingLifetime = 0.15f;
};

class Game
{
public:
    Game();

    void update(float deltaTime, const GameInput& input);
    void damagePlayer(int damage);

    EncounterState getState() const;
    int getPlayerHealth() const;

    const Camera& getCamera() const;
    const std::vector<Enemy>& getEnemies() const;
    const std::vector<Projectile>& getProjectiles() const;
    const std::vector<RaycastTracer>& getRaycastTracers() const;

private:
    void shootRaycast();
    void shootProjectile();
    void updateProjectiles(float deltaTime);
    void updateRaycastTracers(float deltaTime);
    void damageEnemy(Enemy& enemy, int damage);
    void resetEncounter();

    Camera m_camera;
    std::vector<Enemy> m_enemies;
    std::vector<Projectile> m_projectiles;
    std::vector<RaycastTracer> m_raycastTracers;
    EncounterState m_state = EncounterState::Playing;
    int m_playerHealth = 3;
    float m_raycastCooldown = 0.0f;
    float m_projectileCooldown = 0.0f;
};
