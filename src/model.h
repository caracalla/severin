#pragma once

#include <glm/glm.hpp>

#include <vector>

// rotate anything and everything 180 degrees about the x axis
// const glm::vec3 kDefaultRotation{};

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
};

struct Model {
  std::vector<Vertex> vertices;

  static Model createTriangle(); // returns a basic rainbow triangle
	static Model createHexahedron( // build a box
			float width,
			float height,
			float depth,
			glm::vec3 base_color = glm::vec3(1.0f, 1.0f, 1.0f));
	static Model createFromOBJ(
			const std::string& asset_basedir,
			const std::string& file_name);
};
