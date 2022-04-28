#pragma once

#include <input.h>

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
	glm::vec3 position;
	glm::vec3 rotation;
};

struct Scene {
	Player player;
	std::vector<Entity> entities;

	static constexpr float kPlayerMovementIncrement = 0.05;

	void step(
			const std::chrono::microseconds dt,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		player.rotation.x += mouse_state.yOffset; // rotation about x axis
		player.rotation.y += mouse_state.xOffset; // rotation about y axis

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

		glm::mat4 y_rotation = glm::rotate(glm::mat4(1.0f), this->player.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 new_pos = translation * y_rotation;

		player.position.x += new_pos.x;
		player.position.y += new_pos.y;
		player.position.z += new_pos.z;
	}
};
