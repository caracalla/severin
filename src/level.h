#include <model.h>
#include <util.h>

#include <glm/glm.hpp>

#include <fstream>
#include <iostream>
#include <sstream>


void logLevelLoadError(const char* message, std::string line) {
	util::logError("%s: %s\n", message, line.c_str());
}


struct Level {
  struct Platform {
    glm::vec3 position;
		float width;
		float height;
		float depth;
		glm::vec3 start_pos;
		glm::vec3 end_pos;
    Model model;

		Platform(glm::vec3 start_pos, glm::vec3 end_pos) {
			// figure out position
			width = end_pos.x - start_pos.x;
			height = end_pos.y - start_pos.y;
			depth = end_pos.z - start_pos.z;

			position.x = start_pos.x + (width / 2);
			position.y = start_pos.y + (height / 2);
			position.z = start_pos.z + (depth / 2);

			model = Model::createHexahedron(width, height, depth);
		}
  };

  struct Fighter {
		struct Dimensions {
			float height;
			float width;
			float eye_height;
		};

    glm::vec3 position;
    glm::vec3 rotation;
		Dimensions dimensions;
		Model model;

		Fighter(Dimensions dims, glm::vec3 pos, glm::vec3 rot) {
			position = pos;
			rotation = rot;
			dimensions = dims;

			glm::vec3 color(0.0f, 0.0f, 1.0f);

			model = Model::createHexahedron(dims.width, dims.height, dims.width, color);

			// stupid hack to put the origin of the model at the bottom
			for (auto& vertex : model.vertices) {
				vertex.position.y += dimensions.height / 2;
			}
		}
  };

  std::vector<Platform> platforms;
  std::vector<Fighter> fighters;
	float fighter_height;
	float fighter_width;
	float fighter_eye_height;
	Model fighter_model;
	
	bool is_valid = false;

  static Level loadFromFile(const std::string& level_filename) {
    // open file
    std::ifstream level_file(level_filename);
		Level level;

    if (!level_file.is_open()) {
      util::logError("couldn't load level file %s!\n", level_filename.c_str());
      return level;
		}

		Fighter::Dimensions fighter_dims;
		bool got_fighter_dimensions = false;

		std::string line;

		while (std::getline(level_file, line)) {
			if (line.size() == 0 || line[0] == '#') {
				// skip blank lines and comments
				continue;
			}

			std::istringstream line_stream(line);
			char first_char;
			line_stream >> first_char;

			if (first_char == 'i') {
				// shared fighter dimensions
				if (!(line_stream >> fighter_dims.height >> fighter_dims.width >> fighter_dims.eye_height)) {
					logLevelLoadError("fighter shared info improperly formatted!", line);
					return level;
				}

				got_fighter_dimensions = true;
			} else if (first_char == 'f') {
				// a fighter
				if (!got_fighter_dimensions) {
					logLevelLoadError("need shared fighter info before loading fighters!", line);
					return level;
				}

				glm::vec3 position;
				glm::vec3 rotation;

				if (!(line_stream >> position.x >> position.y >> position.z)) {
					logLevelLoadError("fighter position improperly formatted!", line);
					return level;
				}

				if (!(line_stream >> rotation.x >> rotation.y >> rotation.z)) {
					logLevelLoadError("fighter rotation improperly formatted!", line);
					return level;
				}

				level.fighters.emplace_back(fighter_dims, position, rotation);
			} else if (first_char == 'p') {
				// a platform
				glm::vec3 start_pos;
				glm::vec3 end_pos;

				if (!(line_stream >> start_pos.x >> start_pos.y >> start_pos.z)) {
					logLevelLoadError("platform start position improperly formatted!", line);
					return level;
				}

				if (!(line_stream >> end_pos.x >> end_pos.y >> end_pos.z)) {
					logLevelLoadError("platform end position improperly formatted!", line);
					return level;
				}

				level.platforms.emplace_back(start_pos, end_pos);
			} else {
				logLevelLoadError("unrecognized input", line);
			}
		}

		// validation
		if (level.fighters.size() == 0) {
			logLevelLoadError("no fighters found in level", level_filename);
			return level;
		}
		
		 if (level.platforms.size() == 0) {
			logLevelLoadError("no platforms found in level", level_filename);
			return level;
		}

		// build the fighter model

		// we're done
		level.is_valid = true;

		return level;
  }
};





// bool Engine::loadLevelFile(const std::string& level_filename) {
// 	bool player_found = false;
// 	bool assets_found = false;
// 	std::string assets_basedir{};
// 	uint16_t last_model_id = 0;
// 	uint16_t default_material_id = 0; // placeholder

// 	std::string line;
// 	int line_count = 0;

// 	while (std::getline(level_file, line)) {
// 		if (line.size() == 0 || line[0] == '#') {
// 			// skip blank lines and comments
// 			continue;
// 		}

// 		std::istringstream line_stream(line);

// 		if (line_count == 0) {
// 			// player info
// 			glm::vec3 player_pos;

// 			if (!(line_stream >> player_pos.x >> player_pos.y >> player_pos.z)) {
// 				util::logError("player position improperly formatted!");
// 				return false;
// 			}

// 			_scene->player.position = player_pos;

// 			player_found = true;
// 		} else if (line_count == 1) {
// 			// assets base directory
// 			if (!(line_stream >> assets_basedir)) {
// 				util::logError("assets base directory could not be read!");
// 				return false;
// 			}

// 			assets_found = true;
// 		} else {
// 			char identifier;
// 			line_stream >> identifier;
			
// 			if (identifier == 'm') {
// 				// model
// 				std::string model_file_name;
// 				if (!(line_stream >> model_file_name)) {
// 					util::logError("model file name could not be read: %s", line.c_str());
// 					return false;
// 				}

// 				Model model = Model::createFromOBJ(assets_basedir, model_file_name);
// 				last_model_id = _renderer->uploadModel(std::move(model));
// 			} else if (identifier == 'e') {
// 				// entity
// 				glm::vec3 entity_pos;
// 				glm::vec3 entity_rot;
// 				float entity_scale;

// 				if (
// 						!(
// 								line_stream >> entity_pos.x >> entity_pos.y >> entity_pos.z
// 										>> entity_rot.x >> entity_rot.y >> entity_rot.z
// 										>> entity_scale)) {
// 					util::logError("entity info improperly formatted: %s", line.c_str());
// 					return false;
// 				}

// 				_scene->entities.emplace_back(
// 						last_model_id,
// 						default_material_id,
// 						entity_pos,
// 						entity_rot,
// 						entity_scale);
// 			} else if (identifier == 'h') {
// 				// entity
// 				glm::vec3 hex_min;
// 				glm::vec3 hex_max;
// 				glm::vec3 hex_pos = glm::vec3(0.0); // boxes are always at 0 for now
// 				glm::vec3 hex_rot = glm::vec3(0.0);
// 				float hex_scale = 1.0;

// 				if (
// 						!(
// 								line_stream >> hex_min.x >> hex_min.y >> hex_min.z
// 										>> hex_max.x >> hex_max.y >> hex_max.z)) {
// 					util::logError("hexahedron info improperly formatted: %s", line.c_str());
// 					return false;
// 				}

// 				Model model = Model::createHexahedron(hex_min, hex_max);
// 				uint16_t model_id = _renderer->uploadModel(model);

// 				_scene->entities.emplace_back(
// 						model_id,
// 						default_material_id,
// 						hex_pos,
// 						hex_rot,
// 						hex_scale);
// 			} else {
// 				util::logError("what in the world is this? %s", line.c_str());
// 				return false;
// 			}
// 		}

// 		line_count += 1;
// 	}

// 	util::log("successfully loaded level %s", level_filename.c_str());

// 	return true;
// }
