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
{
	resetEncounter();
}

void Game::update(float deltaTime, const GameInput& input)
{
	if (input.restart)
	{
		resetEncounter();
		return;
	}
	if (m_state != EncounterState::Playing)
	{
		return;
	}

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
	m_enemies.erase(std::remove_if(m_enemies.begin(), m_enemies.end(),
		[](const Enemy& enemy) { return enemy.health <= 0; }), m_enemies.end());
	if (m_enemies.empty())
	{
		m_state = EncounterState::Victory;
	}
}

EncounterState Game::getState() const
{
	return m_state;
}

int Game::getPlayerHealth() const
{
	return m_playerHealth;
}

void Game::damagePlayer(int damage)
{
	if (m_state != EncounterState::Playing || damage <= 0)
	{
		return;
	}
	m_playerHealth = std::max(0, m_playerHealth - damage);
	if (m_playerHealth == 0)
	{
		m_state = EncounterState::Defeat;
	}
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
		if (enemy.health <= 0)
		{
			continue;
		}
		const float hitDistance = raySphereHitDistance(
			origin,
			direction,
			enemy.transform.position,
			enemy.getCollisionRadius(),
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
		if (projectile.remainingLifetime <= 0.0f)
		{
			continue;
		}

		// Sweep only the portion of this frame for which the projectile is alive.
		const float travelTime = std::min(deltaTime, projectile.remainingLifetime);
		const glm::vec3 start = projectile.transform.position;
		const glm::vec3 displacement = projectile.velocity * travelTime;
		const float travelDistance = glm::length(displacement);
		const glm::vec3 direction = travelDistance > 0.0f
			? displacement / travelDistance : glm::vec3(0.0f);
		Enemy* closestEnemy = nullptr;
		float closestDistance = travelDistance;

		for (Enemy& enemy : m_enemies)
		{
			if (enemy.health <= 0)
			{
				continue;
			}
			const glm::vec3 enemyOffset = start - enemy.transform.position;
			// Starting inside a sphere is an immediate hit, even without movement.
			float hitDistance = -1.0f;
			const float radius = enemy.getCollisionRadius();
			if (glm::dot(enemyOffset, enemyOffset) <= radius * radius)
			{
				hitDistance = 0.0f;
			}
			else if (travelDistance > 0.0f)
			{
				hitDistance = raySphereHitDistance(
					start, direction, enemy.transform.position,
					radius, closestDistance);
			}

			if (hitDistance >= 0.0f && (!closestEnemy || hitDistance < closestDistance))
			{
				closestEnemy = &enemy;
				closestDistance = hitDistance;
			}
		}

		projectile.transform.position = start + direction * closestDistance;
		projectile.remainingLifetime -= deltaTime;
		if (closestEnemy)
		{
			projectile.remainingLifetime = 0.0f;
			damageEnemy(*closestEnemy, projectile.damage);
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
		std::cout << "Enemy " << enemy.id << " destroyed.\n";
	}
}

void Game::resetEncounter()
{
	m_camera = Camera{};
	m_playerHealth = 3;
	m_state = EncounterState::Playing;
	m_raycastCooldown = 0.0f;
	m_projectileCooldown = 0.0f;
	m_projectiles.clear();
	m_raycastTracers.clear();
	m_enemies.clear();

	Enemy enemy;
	enemy.id = 1;
	enemy.transform.position = m_camera.getTransform().position + glm::vec3(0.0f, 0.0f, -12.0f);
	m_enemies.push_back(enemy);
}
