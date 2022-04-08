
// Copyright (C) David Dalström 2020
#include <Renderer.h>
#include <GameLoop.h>

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <execution>
#include <fstream>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <cassert>
#include <chrono>
#include <functional>
#include <random>
#include <execution>
#include <unordered_map>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Drawable.hpp>

#include "ECS.h"
#include <Ship.h>
#include <Agent.h>

namespace GameJamAsteroids {
	static const std::default_random_engine generator;
	static const std::uniform_real_distribution<float> normalizedDist(0.0f, 100.f);

	const double PI = glm::atan(1) * 4;

	float mix(const float a, const float b, const float mix) {
		return b * mix + a * (1 - mix);
	}

	int sign(float x) {
		return x >= 0.0f ? 1 : -1;
	}

	void simulation(sf::RenderWindow& window, const size_t width, const size_t height, std::vector<ecs::Vec3f>& positions, std::vector<ecs::Vec3f>& velocities, Ship& ship, const float dt) {
		constexpr float friction = 0.9975f;
		constexpr float gravity = 0.025f;
		constexpr float shipGravityFactor = 0.02f;
		
		// create some gravity wells
		const float dx = 1.0f, dy = 1.0f;
		const sf::Vector3f refPoint{ static_cast<float>(width / 2), static_cast<float>(height / 2), 1.0f };
		std::vector<sf::Vector3f> wells = { refPoint };
		for (int i = 0; i < 0; ++i) {
			wells.push_back(refPoint + sf::Vector3f{ -dx * i, dy * i, 1.0f });
			wells.push_back(refPoint + sf::Vector3f{ dx * i, dy * i, 1.0f });
		}
		
		const auto deviation = sf::Vector3f{ cos(dt) * gravity, sin(dt) * gravity, 0.0f };

		constexpr size_t quota = 1;
		static std::vector<size_t> range(positions.size() / quota);
		auto initRange = [&]() {
			std::generate_n(range.begin(), positions.size() / quota, [n = 0]() mutable { return n++; });
		};
		static std::once_flag once;
		std::call_once(once, initRange);

		const auto shipPos = ship.position();
		auto shipGravityWell = [&](size_t i) {
			for (size_t k = quota * i, end = (i + 1) * quota; k < end; ++k) {
				auto& pos = positions[k];
				ecs::Vec3f pull = pos - wells[0];
				pull = pull / (pull.x * pull.x + pull.y * pull.y);
				ecs::Vec3f deflect = pos - shipPos;
				deflect = deflect / (1.0f + deflect.x * deflect.x + deflect.y * deflect.y);

				auto& velocity = velocities[k];
				velocity *= friction;
				velocity -= shipGravityFactor * deflect;

				pos += dt * velocity;
				pos.x = pos.x < 2560.f ? (pos.x > 0.0f ? pos.x : 2559.f) : 0.0f;
				pos.y = pos.y < 1440.f ? (pos.y > 0.0f ? pos.y : 1439.f) : 0.0f;
			}
		};

		auto pushPull = [&](size_t i) {
			for (size_t k = quota * i, end = (i + 1) * quota; k < end; ++k) {
				auto& pos = positions[k];
				ecs::Vec3f pull{ 0.f, 0.f, 0.f };
				for (const auto& gravityWell : wells) {
					auto wellDist = pos - gravityWell;
					auto absDist = 1.0f + wellDist.x * wellDist.x + wellDist.y * wellDist.y;
					pull += gravityWell.z * wellDist / absDist;
				}
				ecs::Vec3f deflect = pos - shipPos;
				deflect = deflect / (1.0f + deflect.x * deflect.x + deflect.y * deflect.y);

				auto& velocity = velocities[k];
				velocity *= friction;
				velocity -= gravity * pull;
				velocity += shipGravityFactor * deflect;

				pos += dt * velocity;
				pos.x = pos.x < width ? (pos.x > 0.0f ? pos.x : width - 1 ) : 0.0f;
				pos.y = pos.y < height ? (pos.y > 0.0f ? pos.y : height - 1) : 0.0f;
			}
		};

		std::for_each(std::execution::par, range.begin(), range.end(), pushPull);
		//std::for_each(std::execution::par, range.begin(), range.end(), shipGravityWell);
	}

	void drawQuads(sf::RenderWindow& window, ECS& ecs, float rad) {
		auto& positions = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Position>();
		auto& velocities = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Velocity>();
		auto& sizes = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Size>();
		auto& colors = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Color>();
		auto& angular = ecs.data<ecs::EntityType::Square, ecs::ComponentType::AngularVelocity>();
		auto& entities = ecs.entities<ecs::EntityType::Square>();

		auto normalized = std::bind(normalizedDist, generator);

		auto distanceSqr = [](const sf::Vector2f a, const sf::Vector2f b) -> float {
			auto xdiff = a.x - b.x;
			auto ydiff = a.y - b.y;
			return xdiff * xdiff + ydiff * ydiff;
		};

		std::vector<sf::Vertex> vertices(4 * positions.size());
		auto random = rand() % 1000;

		static bool doStrobe = false;
// 		static int interval = 100;
// 		if (random > interval) {
// 			doStrobe = !doStrobe;
// 			if (interval == 100) {
// 				interval = 10;
// 			}
// 			else interval = 100;
// 		}

		size_t index = 0;
		size_t n = 0;

		const auto cosRad = cos(rad);
		const auto sinRad = sin(rad);

		for (auto& pos : positions) {
			// layout vertices in a quad pattern
			auto& entity = entities[n];
			if (entity.ttl <= 0) {
				pos.y = pos.x = 0.0f;
				velocities[n].x = 0.025f;
				velocities[n].y = 0.001f;
				entity.ttl = 3000 + 20 * normalized();
				continue;
			}
			entity.ttl--;
			auto& pos = positions[n];
			auto& color = colors[n];
			auto& size = sizes[n];
			auto& angle = angular[n];
			auto& velocity = velocities[n++];
			
			
			sf::Uint8 red{ static_cast<sf::Uint8>(color.x) }, green{ static_cast<sf::Uint8>(color.y) }, blue{ static_cast<sf::Uint8>(color.z) };
			sf::Color sfColor = { red, green, blue, 64 };

			if (doStrobe) {
				sf::Vector2f coneEnd{ 1280.0f + 640.0f * cosRad, 720.0f + 360.0f * sinRad };
				sf::Vector2f pos2d{ pos.x, pos.y };
				sf::Vector2f coneCenter{ 1280.0f, 720.0f };
				auto particleDir = coneCenter - pos2d;
				auto coneDir = coneCenter - coneEnd;
				auto dotProd = coneDir.x * particleDir.x + coneDir.y * particleDir.y;
				auto sonarMagnitude = sqrtf(coneDir.x * coneDir.x + coneDir.y * coneDir.y);
				auto pos2dMagnitude = sqrtf(particleDir.x * particleDir.x + particleDir.y * particleDir.y);
				auto angle = acos(dotProd / (sonarMagnitude * pos2dMagnitude));
				if (angle < 0.4f && normalized() < 1.0f && pos2dMagnitude < 700.0f + 6*normalized() && pos2dMagnitude > 20.0f+1 * normalized()) {
					sfColor = sf::Color{ 212, 175, 55 };
				}
			}

			const ecs::Vec3f ul{ pos.x - size.x, pos.y - size.y, 0.0f };
			const ecs::Vec3f ur{ pos.x + size.x, pos.y - size.y, 0.0f };
			const ecs::Vec3f lr{ pos.x + size.x, pos.y + size.y, 0.0f };
			const ecs::Vec3f ll{ pos.x - size.x, pos.y + size.y, 0.0f };

			vertices[index++] = sf::Vertex(Ship::rotate2D({ ul.x, ul.y }, rad * angle.z, { pos.x, pos.y }), sfColor);
			vertices[index++] = sf::Vertex(Ship::rotate2D({ ur.x, ur.y }, rad * angle.z, { pos.x, pos.y }), sfColor);
			vertices[index++] = sf::Vertex(Ship::rotate2D({ lr.x, lr.y }, rad * angle.z, { pos.x, pos.y }), sfColor);
			vertices[index++] = sf::Vertex(Ship::rotate2D({ ll.x, ll.y }, rad * angle.z, { pos.x, pos.y }), sfColor);
		}

		window.draw(&vertices[0], vertices.size(), sf::Quads);
	}

	void drawCircles(sf::RenderWindow& window, const std::vector<ecs::Vec3f>& positions, std::vector<ecs::Vec3f>& velocities, Ship& ship) {

	}

	void initEntities(size_t width, size_t height, ECS& ecs) {
		std::default_random_engine generator;
		std::uniform_real_distribution<float> normalizedFloatDist(0.0f, 1.0f);
		auto normalizedFloat = std::bind(normalizedFloatDist, generator);

		std::uniform_int_distribution<unsigned int> normalizedDist(0, 100);
		auto normalized = std::bind(normalizedDist, generator);

		auto& positions = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Position>();
		auto& velocities = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Velocity>();
		auto& sizes = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Size>();
		auto& colors = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Color>();
		auto& angular = ecs.data<ecs::EntityType::Square, ecs::ComponentType::AngularVelocity>();
		auto& entities = ecs.entities<ecs::EntityType::Square>();

		for (auto& color : colors) {
			color.x = /*255.f * normalized()*/0.0f;
			color.y = 64.0f + 128.0f * normalizedFloat();
			color.z = /*255.f * normalized()*/0.0f;
		}
		
		for (auto& v : velocities) {
			v.x = 0.01f * normalizedFloat();
			v.y = 0.1f * normalizedFloat();
			v.z = 0.0f;
		}

		for (auto& pos : positions) {
			pos.x = width * normalizedFloat();
			pos.y = height * normalizedFloat();
		}

		const float side = 1.5f;
		for (auto& size : sizes) {
			size.x = side + normalizedFloat() * side;
			size.y = side + normalizedFloat() * side;
			size.z = 0.0f;
		}

		for (auto& angle : angular) {
			angle.x = 0.0f;
			angle.y = 0.0f;
			angle.z = 1.0f + 0.1f * normalizedFloat();
		}

		for (auto& entity : entities) {
			entity.ttl = static_cast<int>(500 + 10*normalized());
		}
	}

	void handleKeyboardEvent(sf::Event event, Ship& ship, sf::RenderWindow& window) {
		if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
			window.close();
		}
		ship.handleKeyboardEvent(event);
	}

	void runGame(size_t width, size_t height) {
		std::random_device randomDevice;
		std::default_random_engine randomEngine(randomDevice());
		std::uniform_int_distribution<int> uniform_dist(0, INT_MAX);

		constexpr size_t quadCount = 100000;
		
		std::vector<Ship> ships;
		for (int i = 0; i < 10; ++i) {
			sf::Color color{ static_cast<sf::Uint8>(uniform_dist(randomEngine) % 256), static_cast<sf::Uint8>(uniform_dist(randomEngine) % 256), static_cast<sf::Uint8>(uniform_dist(randomEngine) % 256) };
			ships.emplace_back(ecs::Vec3f{ static_cast<float>(uniform_dist(randomEngine) % width), static_cast<float>(uniform_dist(randomEngine) % height), 0.0f }, color, std::make_unique<MarkovAgent>());
		}
		
		/*
		ECS ecs;
		ecs.createEntity(ecs::EntityType::Square, { 0.0f, 1.0f, 0.0f }, quadCount);
		auto& positions = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Position>();
		auto& velocities = ecs.data<ecs::EntityType::Square, ecs::ComponentType::Velocity>();
		initEntities(width, height, ecs);
		*/

		sf::RenderWindow window(sf::VideoMode(width, height), "Birds of Pray", sf::Style::Default);
		char windowTitle[255] = "Birds of Pray";
		window.setTitle(windowTitle);
		window.setVerticalSyncEnabled(true);
		
		GameLoop loop;
		loop.run(window, ships);
	}
}
