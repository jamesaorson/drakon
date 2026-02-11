#include <drakon/Game.h>

#include <iostream>

struct Game : public drakon::Game {
	// Inherit constructors
	using drakon::Game::Game;

	void tick(const drakon::Delta delta) override
	{
		std::cout << "Ticking game after " << delta << " seconds" << std::endl;
	}
};

int main()
{
	Game game("Hello");
	game.run();
	return 0;
}
