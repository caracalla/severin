#pragma once

#include <glm/glm.hpp>

#include <vector>


using ModelID = uint16_t;


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
	static Model createIcosahedron();
	static Model createFromOBJ(
			const std::string& asset_basedir,
			const std::string& file_name);
};

Model subdivide(Model original);
