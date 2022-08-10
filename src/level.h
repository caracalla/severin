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
		glm::vec3 start_pos;
		glm::vec3 end_pos;
    Model model;

		Platform(glm::vec3 start_pos, glm::vec3 end_pos, glm::vec3 color) :
				start_pos(start_pos), end_pos(end_pos) {
			position = (start_pos + end_pos) * 0.5f;

			glm::vec3 dimensions = end_pos - start_pos;
			model = Model::createHexahedron(dimensions.x, dimensions.y, dimensions.z, color);
		}
  };

  struct Fighter {
		struct Dimensions {
			float height;
			float width;
			float eye_y_offset;
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
		}
  };

  std::vector<Platform> platforms;
  std::vector<Fighter> fighters;
	
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
				if (!(line_stream >> fighter_dims.height >> fighter_dims.width >> fighter_dims.eye_y_offset)) {
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
				glm::vec3 color{1.0f, 1.0f, 1.0f};

				if (!(line_stream >> start_pos.x >> start_pos.y >> start_pos.z)) {
					logLevelLoadError("platform start position improperly formatted!", line);
					return level;
				}

				if (!(line_stream >> end_pos.x >> end_pos.y >> end_pos.z)) {
					logLevelLoadError("platform end position improperly formatted!", line);
					return level;
				}

				if (!(line_stream >> color.x >> color.y >> color.z)) {
					util::log("using default color for platform %s", line.c_str());
				}

				level.platforms.emplace_back(start_pos, end_pos, color);
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
