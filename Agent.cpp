#include <Agent.h>
#include <Ship.h>

unsigned int MarkovAgent::_seed = 0;

MarkovAgent::MarkovAgent()
	: _randomDevice{ std::make_unique<std::random_device>(std::to_string((++MarkovAgent::_seed) * std::chrono::high_resolution_clock::now().time_since_epoch().count())) } {
}

int MarkovAgent::update(float dt, Ship& ship) {
	static SteeringState lastSteeringAction = SteeringState::Continue;
	static SpeedState lastSpeedAction = SpeedState::Continue;

	auto pos = ship.position();
	int posX = static_cast<int>(pos.x);
	int posY = static_cast<int>(pos.y);
	int length = posX + posY;

	std::default_random_engine randomEngine((*_randomDevice)());
	std::uniform_int_distribution<int> uniform_dist(0, 100);

	SteeringState steeringAction = lastSteeringAction;
	const bool doSteering = uniform_dist(randomEngine) < 2;

	if (doSteering) {
		auto action = uniform_dist(randomEngine);
		if (action < 33) {
			steeringAction = SteeringState::Left;
		}
		else if (action < 66) {
			steeringAction = SteeringState::Right;
		}
		else {
			steeringAction = SteeringState::Continue;
		}
	}

	SpeedState speedAction = lastSpeedAction;
	const bool doSpeed = uniform_dist(randomEngine) < 10;

	if (doSpeed) {
		auto action = uniform_dist(randomEngine);
		if (action < 30) {
			speedAction = SpeedState::Increase;
		}
		else if (action < 60) {
			speedAction = SpeedState::Decrease;
		}
		else {
			speedAction = SpeedState::Continue;
		}
	}

	ship.update(dt, steeringAction, speedAction);

	lastSteeringAction = steeringAction;
	lastSpeedAction = speedAction;

	return 0;
}
