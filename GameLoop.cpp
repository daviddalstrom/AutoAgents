#include <GameLoop.h>
#include <Ship.h>
#include <chrono>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>

GameLoop::GameLoop() {
}

int GameLoop::run(sf::RenderWindow& window, std::vector<Ship>& ships) {
	float tick = 0.0f;
	auto time = std::chrono::steady_clock::now();
	while (window.isOpen()) {
		tick += 0.01f;

		sf::Event event;
		while (window.pollEvent(event)) {
			switch (event.type) {
			case sf::Event::Closed:
				window.close();
				break;
			case sf::Event::KeyPressed:
				if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
					window.close();
				}
				for (auto&& ship : ships) {
					//handleKeyboardEvent(event, ship, window);
				}
				break;
			}
		}

		window.clear(sf::Color::Black);

		//drawQuads(window, ecs, tick);
		for (auto&& ship : ships) {
			ship.draw(window);
		}

		window.display();

		// update
		auto now = std::chrono::steady_clock::now() - time;
		//simulation(window, width, height, positions, velocities, ship, now.count() * 0.00001f);
		for (auto&& ship : ships) {
			ship.update(now.count() * 0.00001f);
		}

		time = std::chrono::steady_clock::now();
	}

	return 0;
}
