#pragma once

#include <collision.h>
#include <input.h>
#include <model.h>
#include <util.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <algorithm>


// the in-world representation of any object
struct Entity { // 64 bytes total
	ModelID mesh_id; // identifier for geometry
	uint16_t material_id; // identifier for shading
	glm::vec3 position; // 12 bytes
	glm::vec3 rotation; // 12 bytes
	float scale = 1.0f; // 4 bytes
	Collision collision; // 32 bytes

	Entity(
			ModelID mesh_id,
			uint16_t material_id,
			glm::vec3 position, // no restriction currently on where an entity's "position" is: for most things, it's the absolute center, but for the player, it's at the bottom
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
			ModelID mesh_id,
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
		if (mass > 0.0f) {
			velocity += force * dt_sec / mass;
		}

		position += velocity * dt_sec;
		force = glm::vec3(0.0f);
	}

	void collideWith(Entity other_entity, glm::vec3& sphere_center_end) {
		// assume this has sphere and static ent has aabb
		Collision::Type other_ent_type = other_entity.collision.type;
		Sphere& sphere = collision.shape.sphere;

		if (other_ent_type == Collision::Type::aabb) {
			AABB& box = other_entity.collision.shape.box;

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
				position = collision_point + (collision_direction * sphere.radius);

				if (collision_direction.y > 0.5f) {
					is_on_ground = true;
				}

				// project current velocity along collision direction, and negate the
				// orthogonal component to provide "bounce back"
				glm::vec3 parallel = glm::dot(collision_direction, velocity) * collision_direction;
				glm::vec3 orthogonal = velocity - parallel;
				velocity = orthogonal - parallel * springiness;
			}
		} else if (other_ent_type == Collision::Type::sphere){
			// do sphere-sphere
		}
	}
};
