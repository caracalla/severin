#pragma once

#include <input.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>


struct Entity { // 32 bytes total
	uint16_t mesh_id; // identifier for geometry
	uint16_t material_id; // identifier for shading
	glm::vec3 position; // 12 bytes
	glm::vec3 rotation; // 12 bytes
	float scale = 1.0; // 4 bytes

	Entity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale) :
					mesh_id(mesh_id),
					material_id(material_id),
					position(position),
					rotation(rotation),
					scale(scale) {}
};

struct Player {
	float eye_height;
	// glm::vec3 eye_position; // relative to model origin point
	int entity_index = 0;
	glm::vec3 position;
	glm::vec3 rotation;
};

struct Camera {
	glm::mat4 view;
	glm::mat4 projection;

	static constexpr glm::vec3 kDefaultPosition = glm::vec3(0.0f, 5.0f, 10.0f);
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
		adjusted_pos.y -= 1.5f; // eye stuff

		view = glm::mat4(1.0f);
		view = glm::rotate(view, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::translate(view, adjusted_pos);
	}
};

struct Scene {
	Camera camera;
	Player player;
	std::vector<Entity> entities;

	Scene(Camera cam) : camera(cam) {}

	static constexpr float kPlayerMovementIncrement = 0.05;

	void step(
			const std::chrono::microseconds dt,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		player.rotation.x += mouse_state.yOffset; // rotation about x axis
		player.rotation.y += mouse_state.xOffset; // rotation about y axis

		// prevent breaking spine
		float max_x_angle = glm::half_pi<float>();
		player.rotation.x = std::clamp(player.rotation.x, -max_x_angle, max_x_angle);

		glm::vec4 translation{};

		if (button_states.forward) { // player moves DOWN x to go forward
			translation.z -= kPlayerMovementIncrement;
		}
		if (button_states.reverse) {
			translation.z += kPlayerMovementIncrement;
		}
		if (button_states.right) {
			translation.x += kPlayerMovementIncrement;
		}
		if (button_states.left) {
			translation.x -= kPlayerMovementIncrement;
		}
		if (button_states.rise) {
			translation.y += kPlayerMovementIncrement;
		}
		if (button_states.fall) {
			translation.y -= kPlayerMovementIncrement;
		}

		glm::mat4 y_rotation =
				glm::rotate(glm::mat4(1.0f), player.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 rotated_translation = translation * y_rotation;

		glm::vec3 movement(
				rotated_translation.x,
				rotated_translation.y,
				rotated_translation.z);

		player.position += movement;

		// update player entity
		Entity& player_ent = entities[player.entity_index];
		player_ent.position = player.position;
		// entity only rotates about y, looks weird otherwise
		// not sure why it has to be negated, I guess due to the coordinate axes crap
		player_ent.rotation.y = -player.rotation.y;

		if (button_states.change_camera) {
			camera.update(Camera::kDefaultPosition, Camera::kDefaultRotation);
		} else {
			camera.update(player.position, player.rotation);
		}
	}
};
