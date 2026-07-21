#include "Game.h"

#include <algorithm>
#include <iostream>

namespace
{
constexpr float ShotInterval = 0.2f;
constexpr float ProjectileSpeed = 12.0f;
constexpr float EnemyCollisionRadius = 0.8f;
constexpr float EnemySpawnDistance = 8.0f;
}

Game::Game()
{
    spawnEnemy();
}

void Game::update(float deltaTime, const GameInput& input)
{
    m_camera.look(input.lookHorizontal, input.lookVertical);
    m_camera.move(input.moveForward, input.moveRight, deltaTime);

    m_enemy.transform.rotation.x += 0.4f * deltaTime;
    m_enemy.transform.rotation.y += 1.0f * deltaTime;

    m_shotCooldown = std::max(0.0f, m_shotCooldown - deltaTime);
    if (input.shoot && m_shotCooldown <= 0.0f)
    {
        shoot();
        m_shotCooldown = ShotInterval;
    }

    updateProjectiles(deltaTime);
}

const Camera& Game::getCamera() const
{
    return m_camera;
}

const Enemy& Game::getEnemy() const
{
    return m_enemy;
}

const std::vector<Projectile>& Game::getProjectiles() const
{
    return m_projectiles;
}

void Game::shoot()
{
    const glm::vec3 direction = m_camera.getForwardDirection();

    Projectile projectile;
    projectile.transform.position = m_camera.getTransform().position + direction * 0.6f;
    projectile.transform.rotation = m_camera.getTransform().rotation;
    projectile.transform.scale = glm::vec3(0.12f, 0.12f, 0.3f);
    projectile.velocity = direction * ProjectileSpeed;
    m_projectiles.push_back(projectile);
}

void Game::updateProjectiles(float deltaTime)
{
    bool enemyDestroyed = false;

    for (Projectile& projectile : m_projectiles)
    {
        projectile.transform.position += projectile.velocity * deltaTime;
        projectile.remainingLifetime -= deltaTime;

        const glm::vec3 enemyOffset = projectile.transform.position - m_enemy.transform.position;
        const bool hitEnemy = glm::dot(enemyOffset, enemyOffset) <= EnemyCollisionRadius * EnemyCollisionRadius;
        if (projectile.remainingLifetime > 0.0f && hitEnemy)
        {
            projectile.remainingLifetime = 0.0f;
            m_enemy.health -= projectile.damage;
            std::cout << "Enemy hit. Health: " << std::max(0, m_enemy.health) << '\n';

            if (m_enemy.health <= 0)
            {
                enemyDestroyed = true;
                break;
            }
        }
    }

    m_projectiles.erase(
        std::remove_if(
            m_projectiles.begin(),
            m_projectiles.end(),
            [](const Projectile& projectile)
            {
                return projectile.remainingLifetime <= 0.0f;
            }),
        m_projectiles.end());

    if (enemyDestroyed)
    {
        std::cout << "Enemy destroyed. Spawning replacement.\n";
        spawnEnemy();
    }
}

void Game::spawnEnemy()
{
    m_enemy = Enemy{};
    m_enemy.transform.position =
        m_camera.getTransform().position + m_camera.getForwardDirection() * EnemySpawnDistance;
}
