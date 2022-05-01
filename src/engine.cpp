#include <engine.h>

#include <level.h>
#include <model.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>


bool Engine::loadLevelFile(const std::string& level_filename) {
	std::ifstream level_file(level_filename);
	Level level = Level::loadFromFile(level_filename);

	if (!level.is_valid) {
		util::logError("level %s is not valid", level_filename.c_str());
		return false;
	}

	uint16_t default_material_id = 0; // placeholder

	for (const auto& platform : level.platforms) {
		uint16_t model_id = _renderer->uploadModel(platform.model);
		_scene->entities.emplace_back(
				model_id,
				default_material_id,
				platform.position,
				glm::vec3(0.0f), // rotation
				1.0f); // scale
	}

	// for now, we only care about the first one
	Level::Fighter& player = level.fighters[0];
	_scene->player.position = player.position;

	util::log("successfully loaded level %s", level_filename.c_str());

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

		// lock frame rate at 60 FPS
		constexpr std::chrono::microseconds kMinFrameTime(16000);

		if (frame_duration < kMinFrameTime) {
			auto sleep_time = kMinFrameTime - frame_duration;

			std::this_thread::sleep_for(sleep_time);

			frame_end = steady_clock::now();
			frame_duration = duration_cast<microseconds>(frame_end - frame_start);
		}

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
