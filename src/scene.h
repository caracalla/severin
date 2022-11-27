#pragma once

#include <entity.h>
#include <input.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>


struct Camera {
	glm::mat4 view;
	glm::mat4 projection;

	static constexpr glm::vec3 kDefaultPosition = glm::vec3(0.0f, 3.0f, 10.0f);
	static constexpr glm::vec3 kDefaultRotation = glm::vec3(0.3, 0.45f, 0.0f);

	Camera(float aspect_ratio) {
		projection = glm::perspective(
				glm::radians(45.0f), // FOV
				aspect_ratio,
				0.1f, // zNear
				200.0f); // zFar
		projection [1][1] *= -1;

		update(kDefaultPosition, kDefaultRotation);
	}

	void update(const glm::vec3& position, const glm::vec3& rotation) {
		glm::vec3 adjusted_pos = position * -1.0f; // coordinate axes are basically reversed

		view = glm::mat4(1.0f);
		view = glm::rotate(view, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::translate(view, adjusted_pos);
	}
};

struct Scene;


struct PlayableEntity {
	DynamicEntityID dynamic_ent_id;
	Scene* scene; // ugh I don't like doing this yet again
	glm::vec3 eye_offset; // relative to model origin point, should be scale and rotation aware I guess (right now only y component is used)
	glm::vec3 view_rotation;
	ModelID projectile_model_id;
	ModelID beam_model_id;
	float cooldown_remaining = 0.0f;
	StaticEntityID pointer_ent_id;
	StaticEntityID beam_gun_ent_id;

	static constexpr float kMaxWalkSpeed = 2.0f; // meters per second
	static constexpr float kSprintFactor = 3.0f;
	static constexpr float kAccelerationFactor = 10.0f;
	static constexpr float kWeaponCooldownSec = 0.1f;

	PlayableEntity(
			DynamicEntityID dynamic_ent_id,
			Scene* scene,
			glm::vec3 eye_offset,
			glm::vec3 view_rotation,
			ModelID projectile_model_id) :
					dynamic_ent_id(dynamic_ent_id),
					scene(scene),
					eye_offset(eye_offset),
					view_rotation(view_rotation),
					projectile_model_id(projectile_model_id) {}

	DynamicEntity& getEntity();
	Entity& getPointerEntity();
	Entity& getBeamGunEntity();

	const glm::vec3 eyePosition() const;

	void moveFromInputs(
    const float dt_sec,
    const Input::ButtonStates button_states,
    const Input::MouseState mouse_state);

	// placeholder actions
	void shootBall(const float dt_sec); // shoots a colliding ball
	void applyForceOnBox(bool is_active); // should make a box spin, someday
	void shootBeam(const float dt_sec); // like the ball, but it's a beam

	const glm::vec3 viewDirection() const {
		constexpr glm::vec4 kDefaultView{0.0, 0.0, -1.0f, 0.0f};

		glm::mat4 direction_rotation =
				glm::rotate(glm::mat4(1.0f), -view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		direction_rotation =
				glm::rotate(direction_rotation, -view_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));

		return glm::vec3(direction_rotation * kDefaultView);
	}
};


struct Scene {
	// TODO: limit the size of the entity arrays to this total value
	static constexpr int kMaxEntities = 4096;

	std::vector<Entity> static_entities;
	std::vector<DynamicEntity> dynamic_entities;
	std::vector<PlayableEntity> playable_entities;

	Camera camera;
	int player_entity_index = 0; // only ever one "player" for now

	Scene(Camera cam) : camera(cam) {}

	PlayableEntity& getPlayer() {
		return playable_entities[player_entity_index];
	}

	// physics works as follows:
	// move dynamic objects
	// collide each with static objects
	// collide with each other
	void applyPhysics(const float dt_sec) {
		constexpr glm::vec3 gravity_acceleration{0.0f, -9.8f, 0.0f};

		for (auto& entity : dynamic_entities) {
			// reset collisions
			entity.collisions = glm::vec3(0.0f);

			entity.applyAcceleration(gravity_acceleration);
			entity.move(dt_sec);

			for (auto static_ent : static_entities) {
				entity.position = entity.collideWith(static_ent, entity.position);
			}

			// update collision
			// we're going to want to do this later
			// represent all dynamic entities as stretched (what's the right word here??)
			// spheres (the path of movement), then collide with each the environment,
			// then each other
			Sphere& sphere = entity.collision.shape.sphere;
			sphere.center_start = entity.position;
		}
	}

	void step(
			const std::chrono::microseconds dt,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		PlayableEntity& player = getPlayer();
		float dt_sec = static_cast<float>(dt.count()) / 1000000;

		// limit the amount of time used for physics
		dt_sec = std::min(dt_sec, 0.02f);

		// update velocity, but not position
		player.moveFromInputs(dt_sec, button_states, mouse_state);

		applyPhysics(dt_sec);

		// TODO: fix high speed collision and remove
		DynamicEntity& player_ent = player.getEntity();
		if (player_ent.position.y < -40.0f) {
			// warp to a high-ish point if you go far enough down
			player_ent.position.y = 10;
			player_ent.velocity.y = 0.0f; // -420.0f;
		}

		// do post-step actions
		for (DynamicEntity& ent : dynamic_entities) {
			if (ent.has_post_action) {
				ent.post_action(&ent, dt_sec);
			}
		}

		static bool third_person_cam = false;

		if (button_states.change_camera) {
			third_person_cam = !third_person_cam;
		}

		if (third_person_cam) {
			glm::vec3 camera_position = Camera::kDefaultPosition;
			glm::mat4 rotation =
					glm::rotate(glm::mat4(1.0f), -player.view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			rotation = glm::rotate(rotation, -player.view_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			camera_position = glm::vec3(rotation * glm::vec4(camera_position, 1.0f));
			camera.update(player_ent.position + camera_position, player.view_rotation);
		} else {
			glm::vec3 camera_position = player.eyePosition();
			camera.update(camera_position, player.view_rotation);
		}
	}

	Entity* addStaticEntity(
			const ModelID mesh_id,
			const uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale) {
		static_entities.emplace_back(mesh_id, material_id, position, rotation, scale);

		return &(static_entities.back());
	}

	DynamicEntity* addDynamicEntity(
			const ModelID mesh_id,
			const uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			float mass) {
		dynamic_entities.emplace_back(mesh_id, material_id, position, rotation, scale, mass);

		return &(dynamic_entities.back());
	}

	PlayableEntity* addPlayableEntity(
			const ModelID mesh_id,
			const uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			float mass,
			glm::vec3 eye_offset,
			const ModelID projectile_model_id) {
		/* DynamicEntity* dynamic_ent = */ addDynamicEntity(
				mesh_id,
				material_id,
				position,
				rotation,
				scale,
				mass);

		playable_entities.emplace_back(
				dynamic_entities.size() - 1,
				this,
				eye_offset,
				rotation,
				projectile_model_id);
		
		return &(playable_entities.back());
	}

	Entity& getStaticEntity(StaticEntityID id) {
		return static_entities[id];
	}

	DynamicEntity& getDynamicEntity(DynamicEntityID id) {
		return dynamic_entities[id];
	}

	const Entity* getNextEntity(int entity_index) const {
		if (entity_index < static_entities.size()) {
			return &(static_entities[entity_index]);
		}

		entity_index -= static_entities.size();

		if (entity_index < dynamic_entities.size()) {
			return static_cast<const Entity*>(&(dynamic_entities[entity_index]));
		}

		return nullptr;
	}
};
