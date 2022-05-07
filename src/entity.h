#pragma once

#include <collision.h>
#include <input.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


// the in-world representation of any object
struct Entity { // 64 bytes total
	uint16_t mesh_id; // identifier for geometry
	uint16_t material_id; // identifier for shading
	glm::vec3 position; // 12 bytes
	glm::vec3 rotation; // 12 bytes
	float scale = 1.0f; // 4 bytes
	Collision collision; // 32 bytes

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
	float springiness = 0.0f;
	bool is_on_ground = true;

	DynamicEntity(
			uint16_t mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation,
			float scale,
			float mass) : Entity(mesh_id, material_id, position, rotation, scale), mass(mass) {}

	void initCollision(float radius) {
		collision.type = Collision::Type::sphere;

		collision.shape.sphere.radius = radius;

		glm::vec3 sphere_pos = position + glm::vec3(0.0f, radius, 0.0f);
		collision.shape.sphere.center_start = sphere_pos;
	}

	void applyForce(glm::vec3 new_force) {
		force += new_force;
	}

	void applyAcceleration(glm::vec3 acceleration) {
		force += acceleration * mass;
	}

	void move(const float dt_sec) {
		velocity += force * dt_sec / mass;
		position += velocity * dt_sec;

		if (position.y < -40.0f) {
			// warp to a high-ish point if you go far enough down
			position.y = 10;
			velocity.y = 0.0f; // -420.0f;
		}

		force = glm::vec3(0.0f);
	}

	void collideWith(Entity static_entity) {
		// assume this has sphere and static ent has aabb
		AABB& box = static_entity.collision.shape.box;
		Sphere& sphere = collision.shape.sphere;
		glm::vec3 sphere_center_end = position;
		sphere_center_end.y += sphere.radius;

		glm::vec3 collision_point;
		bool did_collide = Collision::sphereVsAABB(sphere, sphere_center_end, box, collision_point);

		if (did_collide) {
			glm::vec3 collision_direction = sphere_center_end - collision_point;

			while (glm::all(glm::epsilonEqual(collision_direction, glm::vec3(0.0f), util::kEpsilon))) {
				// sphere_center_end is now inside the box, so we'll walk the sphere
				// backwards from the direction it came in, and keep testing it until
				// it's no longer inside
				collision_direction = glm::normalize(sphere.center_start - sphere_center_end);
				sphere_center_end = collision_point + (collision_direction * sphere.radius);
				sphere.center_start = sphere_center_end + (collision_direction * sphere.radius);
				did_collide = Collision::sphereVsAABB(sphere, sphere_center_end, box, collision_point); // is there any use for did_collide here?
				collision_direction = sphere_center_end - collision_point;
			}

			collision_direction = glm::normalize(collision_direction);
			position = collision_point + (collision_direction * sphere.radius);
			position.y -= sphere.radius;

			if (collision_direction.y > 0.5f) {
				is_on_ground = true;
			}

			// project current velocity along collision direction, and negate the
			// orthogonal component to provide "bounce back"
			glm::vec3 parallel = glm::dot(collision_direction, velocity) * collision_direction;
			glm::vec3 orthogonal = velocity - parallel;
			velocity = orthogonal - parallel * springiness;
		}
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

				if (!is_on_ground) {
					scalar_accel = 0;
				}
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
		if (button_states.jump && is_on_ground) {
			constexpr float jump_speed = 0.8f;
			velocity.y += 8.0f;
			is_on_ground = false;
			util::log("jumping!");
		}

		// player model should only rotate about the y axis
		rotation.y = -view_rotation.y;
	}
};
