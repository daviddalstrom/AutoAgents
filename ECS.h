#pragma once

#include <unordered_map>
#include <vector>
#include <glm/vec3.hpp>
#include <EcsTypes.h>

class ECS {
public:
	ECS() = default;

	ecs::EntityHandle createEntity(const ecs::EntityType type, ecs::Vec3f color);
	std::vector<ecs::EntityHandle> createEntity(const ecs::EntityType type, ecs::Vec3f color, size_t count);
	template <ecs::EntityType E, ecs::ComponentType C>
	std::vector<ecs::Vec3f>& data();
	template <ecs::EntityType E>
	std::vector<ecs::EntityHandle>& entities();

private:
	using EntityMap = std::unordered_map<ecs::EntityType, std::vector<ecs::EntityHandle>>;
	using ComponentDataMap = std::unordered_map<ecs::ComponentType, std::vector<ecs::Vec3f>>;
	using ECDataMap = std::unordered_map<ecs::EntityType, ComponentDataMap>;
	EntityMap mEntities;
	ECDataMap mECData;
};

inline ecs::EntityHandle ECS::createEntity(const ecs::EntityType type, ecs::Vec3f color) {
	auto& container = mEntities[type];
	container.emplace_back(type, ecs::TypeIdBand * static_cast<int>(type) + container.size(), 0);
	auto entityId = container.size();
	switch (type) {
	case ecs::EntityType::Particle:
		[[fallthrough]];
	case ecs::EntityType::Square:
		[[fallthrough]];
	case ecs::EntityType::Circle:
		mECData[type][ecs::ComponentType::Position].push_back({ 0.0f, 0.0f, 0.0f });
		mECData[type][ecs::ComponentType::Velocity].push_back({ 0.0f, 0.0f, 0.0f });
		mECData[type][ecs::ComponentType::Color].push_back(color);
		mECData[type][ecs::ComponentType::Size].push_back({ 1.0f, 1.0f, 0.0f });
		mECData[type][ecs::ComponentType::AngularVelocity].push_back({ 0.0f, 0.0f, 0.0001f });
		break;
	default:
		break;
	}
	return container.back();
}

inline std::vector<ecs::EntityHandle> ECS::createEntity(const ecs::EntityType type, ecs::Vec3f color, size_t count) {
	std::vector<ecs::EntityHandle> handles;
	for (; count > 0; --count) {
		handles.push_back(createEntity(type, color));
	}
	return handles;
}

template <ecs::EntityType E, ecs::ComponentType C>
inline std::vector<ecs::Vec3f>& ECS::data() {
	return mECData[E][C];
}

template<ecs::EntityType E>
inline std::vector<ecs::EntityHandle>& ECS::entities() {
	return mEntities[E];
}
