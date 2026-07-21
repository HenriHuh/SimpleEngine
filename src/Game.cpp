#include "Game.h"

#include <algorithm>
#include <cmath>
#include <iostream>

namespace
{
constexpr float RaycastInterval = 0.1f;
constexpr float RaycastRange = 100.0f;
constexpr float RaycastTracerLength = 25.0f;
constexpr int RaycastDamage = 1;
constexpr float ProjectileInterval = 0.8f;
constexpr float ProjectileSpeed = 12.0f;
constexpr int ProjectileDamage = 5;
constexpr float EnemyCollisionRadius = 0.8f;
constexpr int EnemyCount = 5;
constexpr float MinimumSpawnDistance = 8.0f;
constexpr float MaximumSpawnDistance = 16.0f;
constexpr float HorizontalSpawnSpread = 6.0f;
constexpr float VerticalSpawnSpread = 4.0f;
constexpr float MinimumEnemySeparation = 2.0f;

float raySphereHitDistance(
    const glm::vec3& origin,
    const glm::vec3& direction,
    const glm::vec3& sphereCenter,
    float sphereRadius,
    float maximumDistance)
{
    // Substitute the ray equation into the sphere equation and solve the
    // resulting quadratic. Direction is normalized, simplifying the formula.
    const glm::vec3 originToCenter = origin - sphereCenter;
    const float halfB = glm::dot(originToCenter, direction);
    const float c = glm::dot(originToCenter, originToCenter) - sphereRadius * sphereRadius;
    const float discriminant = halfB * halfB - c;
    if (discriminant < 0.0f)
    {
        return -1.0f;
    }

    const float squareRoot = std::sqrt(discriminant);
    const float nearDistance = -halfB - squareRoot;
    const float farDistance = -halfB + squareRoot;
    const float hitDistance = nearDistance >= 0.0f ? nearDistance : farDistance;
    return hitDistance >= 0.0f && hitDistance <= maximumDistance ? hitDistance : -1.0f;
}
}

Game::Game()
    : m_randomGenerator(std::random_device{}())
{
    m_enemies.resize(EnemyCount);
    for (int index = 0; index < EnemyCount; ++index)
    {
        m_enemies[index].id = index + 1;
        spawnEnemy(m_enemies[index]);
    }
}

void Game::update(float deltaTime, const GameInput& input)
{
    m_camera.look(input.lookHorizontal, input.lookVertical);
    m_camera.move(input.moveForward, input.moveRight, input.moveUp, deltaTime);

    for (Enemy& enemy : m_enemies)
    {
        enemy.transform.rotation.x += 0.4f * deltaTime;
        enemy.transform.rotation.y += 1.0f * deltaTime;
    }

    m_raycastCooldown = std::max(0.0f, m_raycastCooldown - deltaTime);
    m_projectileCooldown = std::max(0.0f, m_projectileCooldown - deltaTime);
    updateRaycastTracers(deltaTime);

    if (input.fireRaycast && m_raycastCooldown <= 0.0f)
    {
        shootRaycast();
        m_raycastCooldown = RaycastInterval;
    }

    if (input.fireProjectile && m_projectileCooldown <= 0.0f)
    {
        shootProjectile();
        m_projectileCooldown = ProjectileInterval;
    }

    updateProjectiles(deltaTime);
}

const Camera& Game::getCamera() const
{
    return m_camera;
}

const std::vector<Enemy>& Game::getEnemies() const
{
    return m_enemies;
}

const std::vector<Projectile>& Game::getProjectiles() const
{
    return m_projectiles;
}

const std::vector<RaycastTracer>& Game::getRaycastTracers() const
{
    return m_raycastTracers;
}

void Game::shootRaycast()
{
    const glm::vec3 origin = m_camera.getTransform().position;
    const glm::vec3 direction = m_camera.getForwardDirection();
    Enemy* closestEnemy = nullptr;
    float closestDistance = RaycastRange;

    // Several enemies may overlap the ray, so only damage the first one the
    // ray reaches.
    for (Enemy& enemy : m_enemies)
    {
        const float hitDistance = raySphereHitDistance(
            origin,
            direction,
            enemy.transform.position,
            EnemyCollisionRadius,
            closestDistance);

        if (hitDistance >= 0.0f && hitDistance < closestDistance)
        {
            closestEnemy = &enemy;
            closestDistance = hitDistance;
        }
    }

    if (closestEnemy)
    {
        damageEnemy(*closestEnemy, RaycastDamage);
    }

    RaycastTracer tracer;
    tracer.start = origin + direction * 0.4f;
    tracer.end = origin + direction * (closestEnemy ? closestDistance : RaycastTracerLength);
    m_raycastTracers.push_back(tracer);
}

void Game::shootProjectile()
{
    const glm::vec3 direction = m_camera.getForwardDirection();

    Projectile projectile;
    projectile.transform.position = m_camera.getTransform().position + direction * 0.6f;
    projectile.transform.rotation = m_camera.getTransform().rotation;
    projectile.transform.scale = glm::vec3(0.12f, 0.12f, 0.3f);
    projectile.velocity = direction * ProjectileSpeed;
    projectile.damage = ProjectileDamage;
    m_projectiles.push_back(projectile);
}

void Game::updateProjectiles(float deltaTime)
{
    for (Projectile& projectile : m_projectiles)
    {
        projectile.transform.position += projectile.velocity * deltaTime;
        projectile.remainingLifetime -= deltaTime;

        if (projectile.remainingLifetime <= 0.0f)
        {
            continue;
        }

        for (Enemy& enemy : m_enemies)
        {
            const glm::vec3 enemyOffset = projectile.transform.position - enemy.transform.position;
            const bool hitEnemy = glm::dot(enemyOffset, enemyOffset) <= EnemyCollisionRadius * EnemyCollisionRadius;
            if (hitEnemy)
            {
                projectile.remainingLifetime = 0.0f;
                damageEnemy(enemy, projectile.damage);
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
}

void Game::updateRaycastTracers(float deltaTime)
{
    for (RaycastTracer& tracer : m_raycastTracers)
    {
        tracer.remainingLifetime -= deltaTime;
    }

    m_raycastTracers.erase(
        std::remove_if(
            m_raycastTracers.begin(),
            m_raycastTracers.end(),
            [](const RaycastTracer& tracer)
            {
                return tracer.remainingLifetime <= 0.0f;
            }),
        m_raycastTracers.end());
}

void Game::damageEnemy(Enemy& enemy, int damage)
{
    enemy.health -= damage;
    std::cout << "Enemy " << enemy.id << " hit for " << damage
              << ". Health: " << std::max(0, enemy.health) << '\n';

    if (enemy.health <= 0)
    {
        std::cout << "Enemy " << enemy.id << " destroyed. Spawning replacement.\n";
        spawnEnemy(enemy);
    }
}

void Game::spawnEnemy(Enemy& enemy)
{
    const int enemyId = enemy.id;
    enemy = Enemy{};
    enemy.id = enemyId;

    std::uniform_real_distribution<float> horizontalOffset(-HorizontalSpawnSpread, HorizontalSpawnSpread);
    std::uniform_real_distribution<float> verticalOffset(-VerticalSpawnSpread, VerticalSpawnSpread);
    std::uniform_real_distribution<float> distance(MinimumSpawnDistance, MaximumSpawnDistance);

    const glm::vec3 cameraPosition = m_camera.getTransform().position;
    const glm::mat4 cameraWorld = m_camera.getTransform().getMatrix();

    // Try several random points in front of the camera and prefer one that is
    // not too close to another enemy. The final candidate is still valid if
    // the small set of attempts happens to find only crowded points.
    for (int attempt = 0; attempt < 12; ++attempt)
    {
        const glm::vec4 localOffset{
            horizontalOffset(m_randomGenerator),
            verticalOffset(m_randomGenerator),
            -distance(m_randomGenerator),
            0.0f};
        enemy.transform.position = cameraPosition + glm::vec3(cameraWorld * localOffset);

        const bool overlapsAnotherEnemy = std::any_of(
            m_enemies.begin(),
            m_enemies.end(),
            [&enemy](const Enemy& other)
            {
                if (&other == &enemy)
                {
                    return false;
                }

                const glm::vec3 separation = enemy.transform.position - other.transform.position;
                return glm::dot(separation, separation) < MinimumEnemySeparation * MinimumEnemySeparation;
            });

        if (!overlapsAnotherEnemy)
        {
            break;
        }
    }
}
