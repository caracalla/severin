#include <engine.h>

#include <fstream>
#include <iostream>
#include <sstream>


bool Engine::loadLevelFile(const std::string& level_filename) {
	return true;
}

void Engine::run() {
	using namespace std::chrono;

	steady_clock::time_point frame_start =
			steady_clock::now();

	while (isRunning()) {
		// draw current scene
		_renderer->draw(_scene);
		
		// get frame duration
		steady_clock::time_point frame_end = steady_clock::now();
		microseconds frame_duration = duration_cast<microseconds>(frame_end - frame_start);
		frame_start = frame_end;

		util::logFrameStats(frame_duration);

		// get inputs
		_window_handler->handleInput();
		Input::ButtonStates button_states = _window_handler->getButtonStates();
		Input::MouseState mouse_state = _window_handler->getMouseState();

		// handle movement and stuff
		_scene->step(frame_duration, button_states, mouse_state);
	}

	_renderer->cleanup();
}
