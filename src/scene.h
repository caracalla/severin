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

// this is really just "whatever is being controlled by user input right now"
struct Player {
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 eye_position; // relative to model origin point
	int entity_index = 0;

	static constexpr float kMovementIncrement = 0.05;
	static constexpr float kSprintFactor = 2;

	void moveFromInputs(
			const std::chrono::microseconds dt,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		// apply mouse movement to rotation
		rotation.x += mouse_state.yOffset; // rotation about x axis
		rotation.y += mouse_state.xOffset; // rotation about y axis

		// prevent breaking spine
		float max_x_angle = glm::half_pi<float>();
		rotation.x = std::clamp(rotation.x, -max_x_angle, max_x_angle);

		// apply key inputs to motion
		glm::vec4 translation{};

		float max_velocity = kMovementIncrement;

		if (button_states.sprint) {
			max_velocity *= kSprintFactor;
		}

		if (button_states.forward) { // player moves DOWN x to go forward
			translation.z -= max_velocity;
		}
		if (button_states.reverse) {
			translation.z += max_velocity;
		}
		if (button_states.right) {
			translation.x += max_velocity;
		}
		if (button_states.left) {
			translation.x -= max_velocity;
		}
		if (button_states.rise) {
			translation.y += max_velocity;
		}
		if (button_states.fall) {
			translation.y -= max_velocity;
		}

		glm::mat4 y_rotation =
				glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 rotated_translation = translation * y_rotation;

		position.x += rotated_translation.x;
		position.y += rotated_translation.y;
		position.z += rotated_translation.z;
	}
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

	void step(
			const std::chrono::microseconds dt,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		player.moveFromInputs(dt, button_states, mouse_state);

		// update player entity
		Entity& player_ent = entities[player.entity_index];
		player_ent.position = player.position;
		// entity only rotates about y, looks weird otherwise
		// not sure why it has to be negated, I guess due to the coordinate axes crap
		player_ent.rotation.y = -player.rotation.y;

		glm::vec3 camera_position = player.position + player.eye_position;
		camera.update(camera_position, player.rotation);

		// if (button_states.change_camera) {
		// 	camera.update(Camera::kDefaultPosition, Camera::kDefaultRotation);
		// } else {
		// 	camera.update(player.position, player.rotation);
		// }
	}
};
