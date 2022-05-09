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

	void applyPhysics(const float dt_sec) {
		constexpr glm::vec3 gravity_acceleration{0.0f, -9.8f, 0.0f};

		for (auto& entity : playable_entities) {
			entity.applyAcceleration(gravity_acceleration);
			entity.move(dt_sec);

			entity.is_on_ground = false;
			for (auto static_ent : static_entities) {
				entity.collideWith(static_ent); // may set entity.is_on_ground back to true
			}

			// update collision
			Sphere& sphere = entity.collision.shape.sphere;
			sphere.center_start = entity.position;
			sphere.center_start.y += sphere.radius;
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

		static bool third_person_cam = false;

		// TODO: remove
		if (button_states.change_camera) {
			third_person_cam = !third_person_cam;
		}

		if (third_person_cam) {
			// camera.update(Camera::kDefaultPosition, Camera::kDefaultRotation);
			glm::vec3 camera_position = Camera::kDefaultPosition;
			glm::mat4 rotation =
					glm::rotate(glm::mat4(1.0f), -player.view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			rotation = glm::rotate(rotation, -player.view_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			camera_position = glm::vec3(rotation * glm::vec4(camera_position, 1.0f));
			camera.update(player.position + camera_position, player.view_rotation);
		} else {
			glm::vec3 camera_position = player.position + player.eye_position;
			camera.update(camera_position, player.view_rotation);
		}
	}

	void addStaticEntity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale) {
		static_entities.emplace_back(mesh_id, material_id, position, rotation, scale);
	}

	void addDynamicEntity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			float mass) {
		dynamic_entities.emplace_back(mesh_id, material_id, position, rotation, scale, mass);
	}

	void addPlayableEntity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			glm::vec3 eye_position,
			float mass) {
		playable_entities.emplace_back(mesh_id, material_id, position, rotation, scale, mass, eye_position);
	}

	const Entity* getNextEntity(int entity_index) const {
		if (entity_index < static_entities.size()) {
			return &(static_entities[entity_index]);
		}

		entity_index -= static_entities.size();

		if (entity_index < dynamic_entities.size()) {
			return static_cast<const Entity*>(&(dynamic_entities[entity_index]));
		}

		entity_index -= dynamic_entities.size();

		if (entity_index < playable_entities.size()) {
			return static_cast<const Entity*>(&(playable_entities[entity_index]));
		}

		return nullptr;
	}
};
