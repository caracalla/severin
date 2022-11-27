#pragma once

#include <collision.h>
#include <input.h>
#include <model.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


using StaticEntityID = uint16_t;

// the in-world representation of any object
struct Entity { // 64 bytes total
	ModelID mesh_id; // identifier for geometry
	uint16_t material_id; // identifier for shading
	// for now, position is basically the circumcenter of the object
	glm::vec3 position; // 12 bytes
	// replace (currently Euler angles) rotation with axis-angle
	// do this because the way I'm currently handling rotations is broken
	glm::vec3 rotation_euler; // 12 bytes
	glm::vec3 rotation_axis; // 12 bytes
	float rotation_angle;
	float scale = 1.0f; // 4 bytes
	Collision collision; // 28 bytes
	bool use_euler = false;

	Entity(
			ModelID mesh_id,
			uint16_t material_id,
			glm::vec3 position, // no restriction currently on where an entity's "position" is: for most things, it's the absolute center, but for the player, it's at the bottom
			glm::vec3 rotation_euler,
			float scale) :
					mesh_id(mesh_id),
					material_id(material_id),
					position(position),
					rotation_euler(rotation_euler),
					scale(scale),
					use_euler(true) {}

	Entity(
			ModelID mesh_id,
			uint16_t material_id,
			glm::vec3 position, // no restriction currently on where an entity's "position" is: for most things, it's the absolute center, but for the player, it's at the bottom
			glm::vec3 rotation_axis,
			float rotation_angle,
			float scale) :
					mesh_id(mesh_id),
					material_id(material_id),
					position(position),
					rotation_axis(rotation_axis),
					rotation_angle(rotation_angle),
					scale(scale),
					use_euler(false) {}

	const glm::mat4 getModelMatrix() const {
		glm::mat4 model_matrix = glm::mat4(1.0f);

		if (use_euler) {
			return glm::scale(
					glm::rotate(
							glm::rotate(
									glm::rotate(
											glm::translate(model_matrix, position),
											rotation_euler.y,
											glm::vec3(0.0f, 1.0f, 0.0f)),
									rotation_euler.x,
									glm::vec3(1.0f, 0.0f, 0.0f)),
							rotation_euler.z,
							glm::vec3(0.0f, 0.0f, 1.0f)),
					glm::vec3(scale));
		} else {
			return glm::scale(
					glm::rotate(glm::translate(model_matrix, position), rotation_angle, rotation_axis),
					glm::vec3(scale));
		}

		return model_matrix;
	}
};


using DynamicEntityID = uint16_t;
struct DynamicEntity;
using EntityAction = std::function<void(DynamicEntity* self, const float dt_sec)>;

struct DynamicEntity : public Entity {
	glm::vec3 collisions{0.0f}; // a sum of the direction of collision with each entity
	glm::vec3 velocity{0.0f};
	glm::vec3 force{0.0f};
	// glm::vec3 angular_velocity{0.0f};
	// glm::vec3 torque{0.0f};
	float mass;
	float springiness = 0.0f;

	// actions
	bool has_post_action = false;
	EntityAction post_action;

	// init functions
	DynamicEntity(
			ModelID mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation_euler,
			float scale,
			float mass) : Entity(mesh_id, material_id, position, rotation_euler, scale), mass(mass) {}

	DynamicEntity(
			ModelID mesh_id,
			uint16_t material_id,
			glm::vec3 position,
			glm::vec3 rotation_axis,
			float rotation_angle,
			float scale,
			float mass) : Entity(mesh_id, material_id, position, rotation_axis, rotation_angle, scale), mass(mass) {}

	void initCollision(const float radius) {
		// always a sphere for now
		collision.type = Collision::Type::sphere;

		collision.shape.sphere.radius = radius;

		glm::vec3 sphere_pos = position + glm::vec3(0.0f, radius, 0.0f);
		collision.shape.sphere.center_start = sphere_pos;
	}

	void setPostAction(EntityAction action) {
		has_post_action = true;
		post_action = action;
	}

	// mid-action functions
	void applyForce(const glm::vec3 new_force) {
		force += new_force;
	}

	void applyAcceleration(const glm::vec3 acceleration) {
		force += acceleration * mass;
	}

	void move(const float dt_sec) {
		if (mass > 0.0f) {
			velocity += force * dt_sec / mass;
		}

		position += velocity * dt_sec;
		force = glm::vec3(0.0f);
	}

	const glm::vec3 collideWith(const Entity other_entity, glm::vec3& sphere_center_end) {
		// assume this has sphere and static ent has aabb
		const Collision::Type other_ent_type = other_entity.collision.type;
		Sphere& sphere = collision.shape.sphere;
		glm::vec3 new_position = sphere_center_end;

		if (other_ent_type == Collision::Type::none) {
			// do nothing
		} else if (other_ent_type == Collision::Type::aabb) {
			const AABB& box = other_entity.collision.shape.box;

			glm::vec3 collision_point;
			bool did_collide = Collision::sphereVsAABB(sphere, sphere_center_end, box, collision_point);

			if (did_collide) {
				glm::vec3 collision_direction = sphere_center_end - collision_point;

				while (util::isVectorZero(collision_direction)) {
					// sphere_center_end is now inside the box, so we'll walk the sphere
					// backwards from the direction it came in, and keep testing it until
					// it's no longer inside
					collision_direction = sphere.center_start - sphere_center_end;

					if (util::isVectorZero(collision_direction)) {
						// just move in a random direction
						collision_direction = glm::vec3(0.0f, 0.0f, 1.0f);
					} else {
						collision_direction = glm::normalize(collision_direction);
					}

					sphere_center_end = collision_point + (collision_direction * sphere.radius);
					sphere.center_start = sphere_center_end + (collision_direction * sphere.radius);
					did_collide = Collision::sphereVsAABB(sphere, sphere_center_end, box, collision_point); // is there any use for did_collide here?
					collision_direction = sphere_center_end - collision_point;
				}

				collision_direction = glm::normalize(collision_direction);
				new_position = collision_point + (collision_direction * sphere.radius);

				// project current velocity along collision direction, and negate the
				// orthogonal component to provide "bounce back"
				glm::vec3 parallel = glm::dot(collision_direction, velocity) * collision_direction;
				glm::vec3 orthogonal = velocity - parallel;
				velocity = orthogonal - parallel * springiness;

				collisions += collision_direction;
			}
		} else if (other_ent_type == Collision::Type::sphere) {
			const Sphere& other_sphere = other_entity.collision.shape.sphere;

			glm::vec3 separation = sphere_center_end - other_sphere.center_start;
			glm::vec3 collision_direction;
			float distance;
			float final_distance = sphere.radius + other_sphere.radius;
			
			if (util::isVectorZero(separation)) {
				distance = 0.0f;

				if (util::isVectorZero(velocity)) {
					// arbitrary direction for this edge case
					collision_direction = glm::vec3(0.0f, 0.0f, 1.0f);
				} else {
					// just go backwards
					collision_direction = glm::normalize(-velocity);
				}
			} else {
				distance = glm::length(separation);
				collision_direction = glm::normalize(separation);
			}

			if (distance < final_distance) {
				// did collide
				new_position = other_sphere.center_start + final_distance * collision_direction;

				glm::vec3 parallel = glm::dot(collision_direction, velocity) * collision_direction;
				glm::vec3 orthogonal = velocity - parallel;
				velocity = orthogonal - parallel * springiness;

				collisions += collision_direction;
			}
		}

		return new_position;
	}

	// post-action functions (only call after physics and collisions have taken place)
	const bool didCollide() const {
		return !util::isVectorZero(collisions);
	}

	const glm::vec3 collisionDirection() const {
		return util::safeNormalize(collisions);
	}

	// used only for player jumping physics right now
	const bool isOnGround() const {
		return collisionDirection().y > 0.5f;
	}
};
