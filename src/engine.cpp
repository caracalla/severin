#include <engine.h>

#include <fstream>
#include <iostream>
#include <sstream>


bool Engine::loadLevelFile(const std::string& level_filename) {
	std::ifstream level_file(level_filename);

	if (!level_file.is_open()) {
		util::logError("couldn't load level file %s!\n", level_filename.c_str());
		return false;
	}

	bool player_found = false;
	bool assets_found = false;
	std::string assets_basedir{};
	uint16_t last_model_id = 0;
	uint16_t default_material_id = 0; // placeholder

	std::string line;
	int line_count = 0;

	while (std::getline(level_file, line)) {
		if (line.size() == 0 || line[0] == '#') {
			// skip blank lines and comments
			continue;
		}

		std::istringstream line_stream(line);

		if (line_count == 0) {
			// player info
			glm::vec3 player_pos;

			if (!(line_stream >> player_pos.x >> player_pos.y >> player_pos.z)) {
				util::logError("player position improperly formatted!");
				return false;
			}

			_scene->player.position = player_pos;

			player_found = true;
		} else if (line_count == 1) {
			// assets base directory
			if (!(line_stream >> assets_basedir)) {
				util::logError("assets base directory could not be read!");
				return false;
			}

			assets_found = true;
		} else {
			char identifier;
			line_stream >> identifier;
			
			if (identifier == 'm') {
				// model
				std::string model_file_name;
				if (!(line_stream >> model_file_name)) {
					util::logError("model file name could not be read: %s", line.c_str());
					return false;
				}

				last_model_id = _renderer->uploadOBJModel(assets_basedir, model_file_name);
			} else if (identifier == 'e') {
				// entity
				glm::vec3 entity_pos;
				glm::vec3 entity_rot;
				float entity_scale;

				if (
						!(
								line_stream >> entity_pos.x >> entity_pos.y >> entity_pos.z
										>> entity_rot.x >> entity_rot.y >> entity_rot.z
										>> entity_scale)) {
					util::logError("entity info improperly formatted: %s", line.c_str());
					return false;
				}

				_scene->entities.emplace_back(
						last_model_id,
						default_material_id,
						entity_pos,
						entity_rot,
						entity_scale);
			} else {
				util::logError("what in the world is this? %s", line.c_str());
				return false;
			}
		}

		line_count += 1;
	}

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
