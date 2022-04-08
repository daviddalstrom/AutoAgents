#pragma once

#include <SFML/System/Vector3.hpp> 

namespace ecs {

enum class EntityType: int {
	Square,
	Circle,
	Particle
};
struct EntityHandle { 
	EntityHandle(const EntityType t, unsigned int id, unsigned int ttl) : type(t), id(id), ttl(ttl) {}
	EntityType type; 
	unsigned int id{ 0 };
	unsigned int ttl; 
};

enum class ComponentType : int {
	Position,
	Velocity,
	Color,
	Size,
	AngularVelocity
};

constexpr unsigned int TypeIdBand = 1000000;

using Vec3f = sf::Vector3f;

using Position = Vec3f;
using Velocity = Vec3f;
using Color = Vec3f;

struct Square {
	Vec3f position;
	unsigned int side;
};

struct Circle {
	Vec3f center;
	unsigned int radius;
};

struct Particle {
	Vec3f position;
};

}