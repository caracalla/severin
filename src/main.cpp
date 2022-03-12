#include <util.h>
#include <window_handler.h>

#include <cstdio>
#include <cstdlib>


int main(int argc, char* argv[]) {
	int width = 800;
	int height = 600;

	WindowHandler window_handler(width, height);

	while (window_handler.isRunning()) {
		window_handler.handleInput();
		Input::ButtonStates button_states = window_handler.getButtonStates();
		Input::MouseState mouse_state = window_handler.getMouseState();
	}

	window_handler.cleanup();

	util::log("all done");

	return EXIT_SUCCESS;
}
