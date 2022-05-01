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
	glm::vec3 position; // bottom of player
	glm::vec3 rotation;
	// float eye_height;
};

struct Camera {
	glm::vec3 position;
	glm::mat4 view;
	glm::mat4 projection;

	static constexpr glm::vec3 kDefaultPosition = glm::vec3(0.0f, -5.0f, -10.0f);

	Camera(float aspect_ratio) {
		position = kDefaultPosition;
		view = glm::translate(glm::mat4(1.0f), position);
		projection = glm::perspective(
				glm::radians(45.0f), // FOV
				aspect_ratio,
				0.1f, // zNear
				200.0f); // zFar
		projection [1][1] *= -1;
	}

	void update(const glm::vec3& new_position, const glm::vec3& new_rotation) {
		// util::logVec3(new_position);
		// util::logVec3(new_rotation);
		position = new_position * -1.0f; // coordinate axes are basically reversed
		position.y -= 1.5f; // eye stuff

		// move camera but not view
		view = glm::mat4(1.0f);
		view = glm::rotate(view, new_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		view = glm::rotate(view, new_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		view = glm::translate(view, position);
	}
};

struct Scene {
	Player player;
	std::vector<Entity> entities;
	Camera camera;

	Scene(Camera new_camera) : camera(new_camera) {
		player.position = glm::vec3(0.0f);
		player.rotation = glm::vec3(0.0f);

		camera.update(player.position, player.rotation);
	}

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

		camera.update(player.position, player.rotation);
	}
};
