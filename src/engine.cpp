#include <engine.h>

#include <level.h>
#include <model.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>


const bool Engine::loadLevelFile(const std::string& level_filename) const {
	std::ifstream level_file(level_filename);
	Level level = Level::loadFromFile(level_filename);

	if (!level.is_valid) {
		util::logError("level %s is not valid", level_filename.c_str());
		return false;
	}

	uint16_t default_material_id = 0; // placeholder

	// **************************************************************************
	// set up static objects
	// **************************************************************************
	// add the spinny box
	{
		glm::vec3 pos{5.0f, 1.5f, -5.0f};
		float size = 2.0f;
		glm::vec3 dims{size, size, size};
		Model model = Model::createHexahedron(dims.x, dims.y, dims.z);
		ModelID model_id = uploadModel(model);

		Entity* ent = _scene->addStaticEntity(
				model_id,
				default_material_id,
				pos,
				glm::vec3(0.0f), // rotation
				1.0f); // scale
		ent->collision.type = Collision::Type::aabb;
		ent->collision.shape.box.min_pos = pos - dims / 2.0f;
		ent->collision.shape.box.max_pos = pos + dims / 2.0f;
	}

	for (const auto& platform : level.platforms) {
		ModelID model_id = uploadModel(platform.model);

		Entity* ent = _scene->addStaticEntity(
				model_id,
				default_material_id,
				platform.position,
				glm::vec3(0.0f), // rotation
				1.0f); // scale
		ent->collision.type = Collision::Type::aabb;
		ent->collision.shape.box.min_pos = platform.start_pos;
		ent->collision.shape.box.max_pos = platform.end_pos;
	}

	// **************************************************************************
	// set up player(s)
	// **************************************************************************
	// just default to the first fighter as the player for now
	int player_fighter_num = 0;
	float fighter_mass = 72.0f;

	// set up projectile model
	Model projectile_model = Model::createIcosahedron();
	projectile_model = subdivide(projectile_model);
	ModelID projectile_model_id = uploadModel(projectile_model);

	for (const auto& fighter : level.fighters) {
		ModelID model_id = uploadModel(fighter.model);

		glm::vec3 fighter_eye_offset = // temporary
				glm::vec3(
						0.0f,
						fighter.dimensions.eye_y_offset,
						0.0f);

		PlayableEntity* playable_ent = _scene->addPlayableEntity(
				model_id,
				default_material_id,
				fighter.position,
				fighter.rotation,
				1.0f, // scale
				fighter_mass,
				fighter_eye_offset,
				projectile_model_id);
		float radius = fighter.dimensions.height / 2;
		DynamicEntity& player_ent = playable_ent->getEntity();
		player_ent.initCollision(radius);
		// player_ent.velocity.y = -420.0f;
	}

	_scene->player_entity_index = player_fighter_num;

	// **************************************************************************
	// set up misc stuff
	// **************************************************************************
	// // add building model
	// Model model = Model::createFromOBJ("assets/", "large_buildingE.obj");
	// glm::vec3 building_pos{10.0f, 0.0f, -10.0f};
	// ModelID model_id = uploadModel(model);
	// _scene->addStaticEntity(
	// 			model_id,
	// 			default_material_id,
	// 			building_pos,
	// 			glm::vec3(0.0f), // rotation
	// 			1.0f); // scale

	// // add icosahedron model
	// Model icosa_model = Model::createIcosahedron();
	// icosa_model = subdivide(icosa_model);
	// glm::vec3 icosa_pos{0.0, 2.0, -5.0};
	// ModelID icosa_model_id = uploadModel(icosa_model);
	// Entity* ball_ent = _scene->addStaticEntity(
	// 			icosa_model_id,
	// 			default_material_id,
	// 			icosa_pos,
	// 			glm::vec3(0.0f), // rotation
	// 			1.0f); // scale
	// // set up icosahedron collision
	// ball_ent->collision.type = Collision::Type::sphere;
	// ball_ent->collision.shape.sphere.radius = 1.0f;
	// ball_ent->collision.shape.sphere.center_start = icosa_pos;

	PlayableEntity& player = _scene->getPlayer();

	util::log("adding the pointer, which indicates where force is being applied");
	// add pointer model
	glm::vec3 player_force_pointer_color{1.0f, 0.0f, 0.0f};
	Model player_force_pointer_model = Model::createIcosahedron(player_force_pointer_color);
	player_force_pointer_model = subdivide(player_force_pointer_model, player_force_pointer_color);
	glm::vec3 player_force_pointer_pos = player.getEntity().position + player.eye_offset;
	ModelID player_force_pointer_model_id = uploadModel(player_force_pointer_model);
	Entity* player_force_pointer_ent = _scene->addStaticEntity(
				player_force_pointer_model_id,
				default_material_id,
				player_force_pointer_pos,
				glm::vec3(0.0f), // rotation
				0.01f); // scale
	player.pointer_ent_id = _scene->static_entities.size() - 1;

	util::log("successfully loaded level %s", level_filename.c_str());

	// set up collision direction displayer (should this be for all dynamic entities?)
	{
		glm::vec3 col_dir_color{1.0f, 0.0f, 1.0f};
		StaticEntityID first_ent_id = _scene->static_entities.size();
		for (int i = 0; i < PlayableEntity::kNumCollisionDirEnts; i++) {
			Model col_dir_model = Model::createIcosahedron(col_dir_color);
			col_dir_model = subdivide(col_dir_model, col_dir_color);
			ModelID col_dir_model_id = uploadModel(col_dir_model);
			Entity* col_dir_ent = _scene->addStaticEntity(
						col_dir_model_id,
						default_material_id,
						glm::vec3(0.0f), // position
						glm::vec3(0.0f), // rotation
						0.05f); // scale
			player.collision_dir_ent_ids[i] = first_ent_id + i;
		}
	}

	// set up the beam for the player beam gun
	{
		glm::vec3 beam_dims{0.5f, 0.5f, 5.0f};
		Model beam_model = Model::createHexahedron(
				beam_dims.x,
				beam_dims.y,
				beam_dims.z,
				glm::vec3(0.0f, 1.0f, 0.0f));
		ModelID beam_model_id = uploadModel(beam_model);
		player.beam_model_id = beam_model_id;
	}

	return true;
}

void Engine::run(const int frames_to_run) {
	using namespace std::chrono;

	steady_clock::time_point frame_start =
			steady_clock::now();

	// lock frame rate at 60 FPS
	constexpr std::chrono::microseconds kMinFrameTime(16000);

	// do a scene step just to get things set up (like the camera)
	_scene->step(kMinFrameTime, Input::ButtonStates{}, Input::MouseState{});

	int frame_count = 0;

	while (isRunning()) {
		// draw current scene
		_renderer->draw(_scene);
		
		// get frame duration
		steady_clock::time_point frame_end = steady_clock::now();
		microseconds frame_duration = duration_cast<microseconds>(frame_end - frame_start);

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
		const Input::ButtonStates button_states = _window_handler->getButtonStates();
		const Input::MouseState mouse_state = _window_handler->getMouseState();

		// handle movement and stuff
		_scene->step(frame_duration, button_states, mouse_state);
		
		frame_count++;
		if (frames_to_run > 0 && frame_count > frames_to_run) {
			break;
		}
	}

	_renderer->cleanup();
}
