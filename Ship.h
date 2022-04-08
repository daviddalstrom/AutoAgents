#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <ECS.h>
#include <chrono>
#include <random>
#include <string>

struct IAgent;
enum class SteeringState;
enum class SpeedState;

class Ship : protected sf::Drawable {
public:
	Ship(const ecs::Vec3f pos, sf::Color color);
	Ship(const ecs::Vec3f pos, sf::Color color, std::unique_ptr<IAgent> agent);

	void draw(sf::RenderTarget& target, sf::RenderStates states = sf::RenderStates::Default) const override;
	void update(float dt);
	void update(sf::RenderWindow& window, const float dt);
	void update(float dt, SteeringState steeringAction, SpeedState speedAction);
	void handleKeyboardEvent(const sf::Event& event);
	ecs::Vec3f position() const;

	static sf::Vector2f rotate2D(sf::Vector2f point, float angle, sf::Vector2f pivot);

	ecs::Vec3f mPosition;
	sf::Color mColor;
	ecs::Vec3f mVelocity;
	ecs::Vec3f mHead;
	float mAcceleration;
	float mHeading;
	float mSpeed{ 0.0f };

	std::unique_ptr<IAgent> mAgent;
};
