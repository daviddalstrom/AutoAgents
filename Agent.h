#pragma once

#include <vector>
#include <random>
#include <memory>

enum class SteeringState : int {
	Continue,
	Left,
	Right
};

enum class SpeedState : int {
	Continue,
	Increase,
	Decrease
};

class Ship;
struct IAgent {
	virtual int update(float dt, Ship& ship) = 0;
};

class MarkovAgent : public IAgent {
public:
	MarkovAgent();
	int update(float dt, Ship& ship) override;

private:
	static unsigned int _seed;
	// Seed with a real random value, if available
	std::unique_ptr<std::random_device> _randomDevice;
};
