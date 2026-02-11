#include <drakon/Game.h>

#include <chrono>

void drakon::Game::run()
{
	// Init before tracking time
	this->init();

	this->makeWindow();

	auto startTime = std::chrono::steady_clock::now();
	drakon::Delta delta = 0;
	while (this->isRunning) {
		auto currentTime = std::chrono::steady_clock::now();
		std::chrono::duration<drakon::Delta> duration = currentTime - startTime;
		delta = duration.count();
		startTime = currentTime;
		this->processEvents();
		// First frame will always have a near-0 value
		this->tick(delta);
	}
	this->done();
	this->cleanup();
}