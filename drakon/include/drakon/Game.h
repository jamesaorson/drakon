#pragma once

#include <string>

namespace drakon {
#if defined(_WIN32) && !defined(_WIN64) || defined(__i386__) || defined(i386)
	typedef float Delta;
#else
	typedef double Delta;
#endif

	struct Game {
		Game() = default;
		Game(std::string title) : title(std::move(title)) {}

		void run();
		void cleanup();
	protected:
		// OS and render engine specific window creation logic
		int makeWindow();
		void processEvents();
		// Abstract methods to be implemented by consuming party
		virtual void init() {}
		virtual void tick(const Delta delta) = 0;
		virtual void done() {}

		bool isRunning = true;
		std::string title = "Drakon Game";
	};
}