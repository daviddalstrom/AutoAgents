#include <Ship.h>
#include <Agent.h>
#include <string>
#include <memory>
#include <random>
#include <chrono>

Ship::Ship(const ecs::Vec3f pos, sf::Color color)
	: mPosition{ pos }
	, mColor{ color }
	, mVelocity{ 0.0f, 0.0f, 0.0f }
	, mHead{ 0.0f, 1.0f, 0.0f }
	, mAcceleration{ 0.0f }
	, mHeading{ 0.0f }
	, mAgent{ nullptr } {
}

Ship::Ship(const ecs::Vec3f pos, sf::Color color, std::unique_ptr<IAgent> agent)
	: mPosition{ pos }
	, mColor{ color }
	, mVelocity{ 0.0f, 0.0f, 0.0f }
	, mHead{ 0.0f, 1.0f, 0.0f }
	, mAcceleration{ 0.0f }
	, mHeading{ 0.0f }
	, mAgent{ std::move(agent) } {
}

void Ship::draw(sf::RenderTarget& target, sf::RenderStates states) const {
	std::vector<sf::Vertex> vertices;
	constexpr float side = 10.0f;

	// head
	sf::Vector2f tip = rotate2D({ mPosition.x + 2 * side, mPosition.y }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f right = rotate2D({ mPosition.x, mPosition.y + side }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f left = rotate2D({ mPosition.x, mPosition.y - side }, mHeading, { mPosition.x, mPosition.y });

	vertices.push_back(sf::Vertex{ tip, mColor });
	vertices.push_back(sf::Vertex{ right, mColor });
	vertices.push_back(sf::Vertex{ left, mColor });

	// body
	sf::Vector2f ul = rotate2D({ mPosition.x - 1.5f * side, mPosition.y - 2.f * side }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f ur = rotate2D({ mPosition.x, mPosition.y - 2.f * side }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f lr = rotate2D({ mPosition.x, mPosition.y + 2.f * side }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f ll = rotate2D({ mPosition.x - 1.5f * side, mPosition.y + 2.f * side }, mHeading, { mPosition.x, mPosition.y });

	// two triangles makes a rectangle
	vertices.push_back(sf::Vertex{ ul, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ ur, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ ll, sf::Color{128, 128, 128} });

	vertices.push_back(sf::Vertex{ ur, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ lr, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ ll, sf::Color{128, 128, 128} });

	// wings
	sf::Vector2f ltip = rotate2D({ mPosition.x - 1.5f * side, mPosition.y - 3.f * side }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f rtip = rotate2D({ mPosition.x - 1.5f * side, mPosition.y + 3.f * side }, mHeading, { mPosition.x, mPosition.y });

	// left wing
	vertices.push_back(sf::Vertex{ ltip, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ ul, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ ur, sf::Color{128, 128, 128} });

	// right wing
	vertices.push_back(sf::Vertex{ rtip, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ ll, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ lr, sf::Color{128, 128, 128} });

	// tail
	sf::Vector2f tailfront = rotate2D({ mPosition.x - 1.5f * side, mPosition.y }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f tailback = rotate2D({ mPosition.x - 3.f * side, mPosition.y }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f tailleft = rotate2D({ mPosition.x - 3.5f * side, mPosition.y - 1.5f * side }, mHeading, { mPosition.x, mPosition.y });
	sf::Vector2f tailright = rotate2D({ mPosition.x - 3.5f * side, mPosition.y + 1.5f * side }, mHeading, { mPosition.x, mPosition.y });

	vertices.push_back(sf::Vertex{ tailfront, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ tailback, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ tailleft, sf::Color{128, 128, 128} });

	vertices.push_back(sf::Vertex{ tailfront, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ tailback, sf::Color{128, 128, 128} });
	vertices.push_back(sf::Vertex{ tailright, sf::Color{128, 128, 128} });

	target.draw(&vertices[0], vertices.size(), sf::Triangles);
}

void Ship::update(float dt) {
	mAgent->update(dt, *this);
}

void Ship::update(sf::RenderWindow& window, const float dt) {
	constexpr float friction = 0.9999f;

	sf::Vector2i mouse = sf::Mouse::getPosition(window); // window is a sf::Window
	mHeading = static_cast<float>(atan2f(mouse.y - mPosition.y, mouse.x - mPosition.x));

	auto dist = static_cast<float>(pow(mouse.x - mPosition.x, 2) + pow(mouse.y - mPosition.y, 2));

	//mSpeed *= friction;
	mSpeed = dist * 0.0000001f;
	mVelocity.x = mSpeed * cos(mHeading);
	mVelocity.y = mSpeed * sin(mHeading);
	mPosition += dt * mVelocity;

	auto width = window.getSize().x;
	auto height = window.getSize().y;

	mPosition.x = mPosition.x < width ? (mPosition.x > 0.0f ? mPosition.x : width - 1) : 0.0f;
	mPosition.y = mPosition.y < height ? (mPosition.y > 0.0f ? mPosition.y : height - 1) : 0.0f;
}

void Ship::update(float dt, SteeringState steeringAction, SpeedState speedAction) {
	const float accelerationDampening = 0.9;
	mAcceleration *= accelerationDampening;

	const float steeringAmount = 0.1;
	switch (steeringAction) {
	case SteeringState::Left:
		mHeading -= steeringAmount;
		break;
	case SteeringState::Right:
		mHeading += steeringAmount;
		break;
	default: {}
	}

	switch (speedAction) {
	case SpeedState::Increase:
		mAcceleration += 0.000001;
		break;
	case SpeedState::Decrease:
		mAcceleration -= 0.000001;
		break;
	default: {}
	}

	mAcceleration = std::min(0.001f, std::max(-0.001f, mAcceleration));
	mSpeed += mAcceleration * dt;

	if (mSpeed < 0.0f) {
		mSpeed = 0.0f;
		mAcceleration = 0.0f;
	}

	mVelocity.x = mSpeed * cos(mHeading);
	mVelocity.y = mSpeed * sin(mHeading);
	mPosition += dt * mVelocity;

	mPosition.x = mPosition.x < 2560.f ? (mPosition.x > 0.0f ? mPosition.x : 2559.f) : 0.0f;
	mPosition.y = mPosition.y < 1440.f ? (mPosition.y > 0.0f ? mPosition.y : 1439.f) : 0.0f;
}

void Ship::handleKeyboardEvent(const sf::Event& event) {
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		mSpeed += .005f;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		mSpeed *= 0.8f;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
		mHeading -= 0.08f;
	}
	else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
		mHeading += 0.08f;
	}
}

ecs::Vec3f Ship::position() const {
	return mPosition;
}

sf::Vector2f Ship::rotate2D(sf::Vector2f point, float angle, sf::Vector2f pivot) {
	const float s = sin(angle);
	const float c = cos(angle);

	// translate point back to origin:
	point.x -= pivot.x;
	point.y -= pivot.y;

	// rotate point
	const float xnew = point.x * c - point.y * s;
	const float ynew = point.x * s + point.y * c;

	return sf::Vector2f{ xnew + pivot.x, ynew + pivot.y };
}
