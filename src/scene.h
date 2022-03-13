#pragma once

#include <input.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>


struct Player {
	glm::vec3 position;
	glm::vec3 rotation;
};

struct Scene {
	// std::vector<RenderObject> entities;
	Player player;

	static constexpr float kPlayerMovementIncrement = 0.05;

	void step(
			std::chrono::microseconds dt,
			Input::ButtonStates button_states,
			Input::MouseState mouse_state) {
		player.rotation.x += mouse_state.yOffset; // rotation about x axis
		player.rotation.y += mouse_state.xOffset; // rotation about y axis

		glm::vec4 translation{};

		if (button_states.forward) {
			translation.z += kPlayerMovementIncrement;
		}
		if (button_states.reverse) {
			translation.z -= kPlayerMovementIncrement;
		}
		if (button_states.left) {
			translation.x += kPlayerMovementIncrement;
		}
		if (button_states.right) {
			translation.x -= kPlayerMovementIncrement;
		}
		if (button_states.rise) {
			translation.y -= kPlayerMovementIncrement;
		}
		if (button_states.fall) {
			translation.y += kPlayerMovementIncrement;
		}

		glm::mat4 y_rotation = glm::rotate(glm::mat4(1.0f), this->player.rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 new_pos = translation * y_rotation;

    player.position.x += new_pos.x;
    player.position.y += new_pos.y;
    player.position.z += new_pos.z;
	}
};
