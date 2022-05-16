#pragma once

#include <util.h>

#include <glm/glm.hpp>


struct Sphere {
	glm::vec3 center_start; // center position as of the end of the last frame
	float radius;
};

struct AABB {
	glm::vec3 min_pos;
	glm::vec3 max_pos;
};


// real time collision detection pg. 131
static float squaredDistanceToAABB(glm::vec3 point, AABB box) {
	float squared_distance = 0.0;

	// for each axis, count any excess distance outside AABB extents
	for (int i = 0; i < 3; i++) {
		float point_ex = point[i];
		float box_min_ex = box.min_pos[i];
		float box_max_ex = box.max_pos[i];

		if (point_ex < box_min_ex) {
			squared_distance += (box_min_ex - point_ex) * (box_min_ex - point_ex);
		}

		if (point_ex > box_max_ex) {
			squared_distance += (point_ex - box_max_ex) * (point_ex - box_max_ex);
		}
	}

	return squared_distance;
}

// real time collision detection pg. 130
static glm::vec3 closestPointToAABB(glm::vec3 point, AABB box) {
	// clamp point to sides of the box
	// util::logVec3(point);
	for (int i = 0; i < 3; i++) {
		float point_ex = point[i];
		float box_min_ex = box.min_pos[i];
		float box_max_ex = box.max_pos[i];

		if (point_ex < box_min_ex) {
			point[i] = box_min_ex;
		}

		if (point_ex > box_max_ex) {
			point[i] = box_max_ex;
		}
	}

	return point;
}


struct Collision {
	enum class Type {
		sphere,
		aabb
	};

	Type type;
	union Shape {
		Sphere sphere;
		AABB box;
	} shape;

	// two options:
	// 1. sweep AABB by radius, do collision with sphere direction vector, if collided, do collision with collision point and AABB
	static bool sphereVsAABB(
			Sphere sphere,
			glm::vec3 sphere_center_end,
			AABB box,
			glm::vec3& collision_point) {
		// float squared_distance = squaredDistanceToAABB(sphere.center_pos, box);
		// float squared_sphere_radius = sphere.radius * sphere.radius;

		// return squared_distance < squared_sphere_radius;

		collision_point = closestPointToAABB(sphere_center_end, box);
		glm::vec3 center_to_closest = collision_point - sphere_center_end;
		// util::logVec3(center_to_closest);
		float squared_distance_to_center = glm::dot(center_to_closest, center_to_closest);
		float squared_sphere_radius = sphere.radius * sphere.radius;
		// util::log(
		// 		"squared dist to center: %f, squared radius: %f, is greater? %d",
		// 		squared_distance_to_center,
		// 		squared_sphere_radius,
		// 		squared_distance_to_center > squared_sphere_radius);
		return squared_distance_to_center <= squared_sphere_radius;
	}
};
