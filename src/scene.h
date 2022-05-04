#pragma once

#include <input.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <vector>


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

		if (position.y < 0.001) {
			position.y = 0;
			velocity = glm::vec3(0.0f);
		}

		force = glm::vec3(0.0f);
	}
};

struct PlayableEntity : public DynamicEntity {
	glm::vec3 eye_position; // relative to model origin point, should be scale and rotation aware I guess (right now only y component is used)
	glm::vec3 view_rotation;

	static constexpr float kWalkSpeed = 1.8f; // meters per second
	static constexpr float kSprintFactor = 3.0f;

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

		// apply key inputs to motion
		glm::vec4 translation{};

		float max_velocity = kWalkSpeed * dt_sec;

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

		constexpr glm::vec3 jump_acceleration{0.0f, 500.0f, 0.0f};
		if (button_states.jump && position.y < 0.001) {
			applyAcceleration(jump_acceleration);
		}

		glm::mat4 y_rotation =
				glm::rotate(glm::mat4(1.0f), view_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		glm::vec4 rotated_translation = translation * y_rotation;

		position.x += rotated_translation.x;
		position.y += rotated_translation.y;
		position.z += rotated_translation.z;

		// player model should only rotate about the y axis
		rotation.y = -view_rotation.y;
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
		}
	}

	void step(
			const std::chrono::microseconds dt,
			const Input::ButtonStates button_states,
			const Input::MouseState mouse_state) {
		PlayableEntity& player = getPlayer();
		float dt_sec = static_cast<float>(dt.count()) / 1000000;

		player.moveFromInputs(dt_sec, button_states, mouse_state);

		applyPhysics(dt_sec);

		// TODO: remove
		if (button_states.change_camera) {
			camera.update(Camera::kDefaultPosition, Camera::kDefaultRotation);
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
