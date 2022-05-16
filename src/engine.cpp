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
		ModelID model_id = _renderer->uploadModel(platform.model);
		_scene->addStaticEntity(
				model_id,
				default_material_id,
				platform.position,
				glm::vec3(0.0f), // rotation
				1.0f); // scale

		Entity& ent = _scene->static_entities.back();
		ent.collision.type = Collision::Type::aabb;
		ent.collision.shape.box.min_pos = platform.start_pos;
		ent.collision.shape.box.max_pos = platform.end_pos;
	}

	// just default to the first fighter as the player for now
	int player_fighter_num = 0;
	float fighter_mass = 72.0f;

	// set up projectile model
	Model projectile_model = Model::createIcosahedron();
	projectile_model = subdivide(projectile_model);
	ModelID projectile_model_id = _renderer->uploadModel(projectile_model);

	for (const auto& fighter : level.fighters) {
		ModelID model_id = _renderer->uploadModel(fighter.model);

		glm::vec3 fighter_eye_position = // temporary
				glm::vec3(
						0.0f,
						fighter.dimensions.eye_height,
						0.0f);

		_scene->addPlayableEntity(
				model_id,
				default_material_id,
				fighter.position,
				fighter.rotation,
				1.0f, // scale
				fighter_eye_position,
				fighter_mass,
				projectile_model_id);

		PlayableEntity& ent = _scene->playable_entities.back();
		float radius = fighter.dimensions.height / 2;
		ent.initCollision(radius);
		// ent.velocity.y = -420.0f;
	}

	_scene->player_entity_index = player_fighter_num;

	// add building model
	Model model = Model::createFromOBJ("assets/", "large_buildingE.obj");
	glm::vec3 building_pos{10.0f, 0.0f, -10.0f};
	ModelID model_id = _renderer->uploadModel(model);
	_scene->addStaticEntity(
				model_id,
				default_material_id,
				building_pos,
				glm::vec3(0.0f), // rotation
				1.0f); // scale

	// add icosahedron model
	Model icosa_model = Model::createIcosahedron();
	icosa_model = subdivide(icosa_model);
	glm::vec3 icosa_pos{0.0, 2.0, -5.0};
	ModelID icosa_model_id = _renderer->uploadModel(icosa_model);
	Entity* ball_ent = _scene->addStaticEntity(
				icosa_model_id,
				default_material_id,
				icosa_pos,
				glm::vec3(0.0f), // rotation
				1.0f); // scale

	// set up icosahedron collision
	ball_ent->collision.type = Collision::Type::sphere;
	ball_ent->collision.shape.sphere.radius = 1.0f;
	ball_ent->collision.shape.sphere.center_start = icosa_pos;

	util::log("successfully loaded level %s", level_filename.c_str());

	return true;
}

void Engine::run(int frames_to_run) {
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
		Input::ButtonStates button_states = _window_handler->getButtonStates();
		Input::MouseState mouse_state = _window_handler->getMouseState();

		// handle movement and stuff
		_scene->step(frame_duration, button_states, mouse_state);
		
		frame_count++;
		if (frames_to_run > 0 && frame_count > frames_to_run) {
			break;
		}
	}

	_renderer->cleanup();
}
