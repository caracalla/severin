#pragma once

#include <input.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


// the in-world representation of any object
struct Entity { // 32 bytes total
	uint16_t mesh_id; // identifier for geometry
	uint16_t material_id; // identifier for shading
	glm::vec3 position; // 12 bytes
	glm::vec3 rotation; // 12 bytes
	float scale = 1.0f; // 4 bytes

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

struct DynamicEntity : public Entity {
	glm::vec3 velocity{0.0f};
	glm::vec3 force{0.0f};
	float mass;

	DynamicEntity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			float mass) : Entity(mesh_id, material_id, position, rotation, scale), mass(mass) {}

	void applyForce(glm::vec3 new_force) {
		force += new_force;
	}

	void applyAcceleration(glm::vec3 acceleration) {
		force += acceleration * mass;
	}

	void move(const float dt_sec) {
		velocity += force * dt_sec / mass;
		position += velocity * dt_sec;

		// stupid hack, remove
		if (position.y < 0.001) {
			position.y = 0;
			velocity.y = 0.0f;
		}

		force = glm::vec3(0.0f);
	}
};

struct PlayableEntity : public DynamicEntity {
	glm::vec3 eye_position; // relative to model origin point, should be scale and rotation aware I guess (right now only y component is used)
	glm::vec3 view_rotation;

	static constexpr float kMaxWalkSpeed = 2.0f; // meters per second
	static constexpr float kSprintFactor = 3.0f;
	static constexpr float kAccelerationFactor = 10.0f;

	PlayableEntity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			float mass,
			glm::vec3 eye_position) :
					DynamicEntity(mesh_id, material_id, position, rotation, scale, mass),
					eye_position(eye_position),
					view_rotation(rotation) {}

	void moveFromInputs(
			const float dt_sec,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		// apply mouse movement to rotation
		view_rotation.x += mouse_state.yOffset; // rotation about x axis
		view_rotation.y += mouse_state.xOffset; // rotation about y axis

		// prevent breaking spine
		float max_x_angle = glm::half_pi<float>();
		view_rotation.x = std::clamp(view_rotation.x, -max_x_angle, max_x_angle);

		// apply key states to velocity
		glm::vec3 desired_direction{0.0f};

		if (button_states.forward) {
			desired_direction.z -= 1.0f;
		}
		if (button_states.reverse) {
			desired_direction.z += 1.0f;
		}
		if (button_states.left) {
			desired_direction.x -= 1.0f;
		}
		if (button_states.right) {
			desired_direction.x += 1.0f;
		}

		bool movement_input_detected = (desired_direction != glm::vec3{0.0f});
		bool is_already_moving = abs(velocity.x) > 0.0f || abs(velocity.z) > 0.0f;

		float max_speed = kMaxWalkSpeed;

		if (button_states.sprint) {
			max_speed = kMaxWalkSpeed * kSprintFactor;
		}

		float scalar_accel = max_speed * kAccelerationFactor;
		float delta_speed = scalar_accel * dt_sec;

		glm::vec3 current_velocity_xz{velocity.x, 0.0f, velocity.z};
		float current_speed_xz = 0.0f;

		if (is_already_moving) {
			current_speed_xz = glm::length(current_velocity_xz);
		}

		// the meat
		if (!movement_input_detected && current_speed_xz <= delta_speed) {
			// no input, moving slowly enough to just stop
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		} else {
			glm::vec3 accel_direction{0.0f};
			glm::vec3 current_direction = glm::normalize(current_velocity_xz);

			if (!movement_input_detected) {
				// no input, but moving faster than delta_speed -> accel in direction opposite current_direction
				accel_direction = glm::normalize(current_direction) * -1.0f;
			} else {
				// we'll use desired_direction, let's prepare it
				glm::mat4 y_rotation =
						glm::rotate(glm::mat4(1.0f), -view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
				desired_direction = glm::vec3(y_rotation * glm::vec4(desired_direction, 1.0f));
				desired_direction = glm::normalize(desired_direction);

				if (!is_already_moving) {
					// input, not moving -> just accel in desired_direction
					accel_direction = desired_direction;
				} else {
					// input, moving -> accel in direction that is the difference of desired and current
					if (abs(desired_direction.x - current_direction.x) > 0.1) {
						accel_direction = desired_direction - current_direction;
					} else {
						// they're already basically aligned, just go in the desired direction
						accel_direction = desired_direction;
					}
				}
			}

			glm::vec3 acceleration = accel_direction * scalar_accel;
			glm::vec3 new_velocity_xz = current_velocity_xz + (acceleration * dt_sec);
			float new_speed_xz = glm::length(new_velocity_xz);

			if (new_speed_xz > max_speed) {
				new_velocity_xz = glm::normalize(new_velocity_xz) * max_speed;
			}

			velocity.x = new_velocity_xz.x;
			velocity.z = new_velocity_xz.z;
		}

		// jump stuff
		if (button_states.jump && position.y < 0.001) {
			constexpr glm::vec3 jump_acceleration{0.0f, 500.0f, 0.0f};
			applyAcceleration(jump_acceleration);
		}

		// player model should only rotate about the y axis
		rotation.y = -view_rotation.y;
	}
};
