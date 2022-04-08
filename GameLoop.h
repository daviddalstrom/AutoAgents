#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <vector>

class Ship;

class GameLoop {
public:
	GameLoop();
	int run(sf::RenderWindow& window, std::vector<Ship>& ships);
};

