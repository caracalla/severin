#include <model.h>
#include <util.h>

#include <tiny_obj_loader.h>

#include <iostream>



struct Triangle {
	Vertex v0;
	Vertex v1;
	Vertex v2;

	Triangle (glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 color) {
		glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);

		v0.position = p0;
		v0.normal = normal;
		v0.color = color;

		v1.position = p1;
		v1.normal = normal;
		v1.color = color;

		v2.position = p2;
		v2.normal = normal;
		v2.color = color;
	}
};


Model Model::createTriangle() {
	Model model;

	model.vertices.resize(3);

	model.vertices[0].position = { 0.5f, 0.5f, 0.0f};
	model.vertices[1].position = {-0.5f, 0.5f, 0.0f};
	model.vertices[2].position = { 0.0f,-0.5f, 0.0f};

	model.vertices[0].color = { 1.0f, 0.0f, 0.0f};
	model.vertices[1].color = { 0.0f, 1.0f, 0.0f};
	model.vertices[2].color = { 0.0f, 0.0f, 1.0f};

	return model;
}

void insertTriangle(Model& model, Triangle triangle) {
	model.vertices.push_back(triangle.v0);
	model.vertices.push_back(triangle.v1);
	model.vertices.push_back(triangle.v2);
}

Model Model::createHexahedron(float width, float height, float depth, glm::vec3 base_color) {
	glm::vec3 box_min(-width / 2, -height / 2, -depth /2);
	glm::vec3 box_max(width / 2, height / 2, depth /2);

	Model model;
	// x: -l, +r
	// y: -b, +t
	// z: -f, +r (z points out of the screen)
	glm::vec3 rtf{box_max.x, box_max.y, box_max.z};
	glm::vec3 ltf{box_min.x, box_max.y, box_max.z};
	glm::vec3 lbf{box_min.x, box_min.y, box_max.z};
	glm::vec3 rbf{box_max.x, box_min.y, box_max.z};

	glm::vec3 rtr{box_max.x, box_max.y, box_min.z};
	glm::vec3 ltr{box_min.x, box_max.y, box_min.z};
	glm::vec3 lbr{box_min.x, box_min.y, box_min.z};
	glm::vec3 rbr{box_max.x, box_min.y, box_min.z};

	Vertex vertex;
	glm::vec3 color = base_color * 0.8f;

	// build top
	insertTriangle(model, Triangle(rtf, rtr, ltr, color));
	insertTriangle(model, Triangle(ltr, ltf, rtf, color));

	// build bottom
	insertTriangle(model, Triangle(rbf, lbf, lbr, color));
	insertTriangle(model, Triangle(lbr, rbr, rbf, color));

	// build front (actually this is the rear I messed up the z coords)
	insertTriangle(model, Triangle(rtf, ltf, lbf, color));
	insertTriangle(model, Triangle(lbf, rbf, rtf, color));

	// build rear (actually this is the front I messed up the z coords)
	glm::vec3 red = glm::vec3(0.4f, 0.0f, 0.0f); // make the front red temporarily
	insertTriangle(model, Triangle(rtr, rbr, lbr, red));
	insertTriangle(model, Triangle(lbr, ltr, rtr, red));

	// build left
	insertTriangle(model, Triangle(ltf, ltr, lbr, color));
	insertTriangle(model, Triangle(lbr, lbf, ltf, color));

	// build right
	insertTriangle(model, Triangle(rtf, rbf, rbr, color));
	insertTriangle(model, Triangle(rbr, rtr, rtf, color));

	return model;
}

Model Model::createIcosahedron() {
	float p = (sqrt(5) - 1) / 2;

	glm::vec3 x1{0, -p, 1};
	glm::vec3 x2{0, p, 1};
	glm::vec3 x3{0, p, -1};
	glm::vec3 x4{0, -p, -1};

	glm::vec3 y1{1, 0, p};
	glm::vec3 y2{1, 0, -p};
	glm::vec3 y3{-1, 0, -p};
	glm::vec3 y4{-1, 0, p};

	glm::vec3 z1{-p, 1, 0};
	glm::vec3 z2{p, 1, 0};
	glm::vec3 z3{p, -1, 0};
	glm::vec3 z4{-p, -1, 0};

	Model model;
	
	glm::vec3 color{1.0f, 0.5f, 0.5f};

	insertTriangle(model, Triangle(z1, y3, y4, color));
	insertTriangle(model, Triangle(z1, x3, y3, color));
	insertTriangle(model, Triangle(z1, z2, x3, color));
	insertTriangle(model, Triangle(z1, x2, z2, color));
	insertTriangle(model, Triangle(z1, y4, x2, color));

	insertTriangle(model, Triangle(y4, y3, z4, color));
	insertTriangle(model, Triangle(z4, y3, x4, color));
	insertTriangle(model, Triangle(y3, x3, x4, color));
	insertTriangle(model, Triangle(x4, x3, y2, color));
	insertTriangle(model, Triangle(x3, z2, y2, color));
	insertTriangle(model, Triangle(y2, z2, y1, color));
	insertTriangle(model, Triangle(z2, x2, y1, color));
	insertTriangle(model, Triangle(y1, x2, x1, color));
	insertTriangle(model, Triangle(x2, y4, x1, color));
	insertTriangle(model, Triangle(x1, y4, z4, color));

	insertTriangle(model, Triangle(z3, y2, y1, color));
	insertTriangle(model, Triangle(z3, y1, x1, color));
	insertTriangle(model, Triangle(z3, x1, z4, color));
	insertTriangle(model, Triangle(z3, z4, x4, color));
	insertTriangle(model, Triangle(z3, x4, y2, color));

	return model;
}

glm::vec3 midpoint(glm::vec3 p0, glm::vec3 p1) {
	return glm::vec3(
			(p0.x + p1.x) / 2,
			(p0.y + p1.y) / 2,
			(p0.z + p1.z) / 2);
}

Model subdivide(Model original) {
	Model new_model;
	glm::vec3 color{1.0, 0.5, 0.5};

	float length = glm::length(original.vertices[0].position);

	for (int i = 0; i < original.vertices.size(); i += 3) {
		glm::vec3 v0 = original.vertices[i].position;
		glm::vec3 v1 = original.vertices[i + 1].position;
		glm::vec3 v2 = original.vertices[i + 2].position;

		glm::vec3 v01 = midpoint(v0, v1) * length;
		glm::vec3 v12 = midpoint(v1, v2) * length;
		glm::vec3 v20 = midpoint(v2, v0) * length;

		insertTriangle(new_model, Triangle(v0, v01, v20, color));
		insertTriangle(new_model, Triangle(v01, v1, v12, color));
		insertTriangle(new_model, Triangle(v20, v12, v2, color));
		insertTriangle(new_model, Triangle(v01, v12, v20, color));
	}

	// make the vertex normals point away from the center of the ball, rather
	// than aligned to the triangle
	for (int i = 0; i < original.vertices.size(); i++) {
		Vertex& vert = original.vertices[i];
		vert.normal = glm::normalize(vert.position);
	}

	return new_model;
}

Model Model::createFromOBJ(
		const std::string& asset_basedir,
		const std::string& file_name) {
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	std::string warn;
	std::string err;

	std::string file_path = asset_basedir + file_name;

	tinyobj::LoadObj(
			&attrib,
			&shapes,
			&materials,
			&warn,
			&err,
			file_path.c_str(),
			asset_basedir.c_str());

	if (!warn.empty()) {
		std::cout << "Warning when loading " << file_path << " OBJ file: " << warn << std::endl;
	}
	if (!err.empty()) {
		std::cerr << "Error when loading " << file_path << " OBJ file: " << err << std::endl;

		return Model::createHexahedron(2.0f, 2.0f, 2.0f);
	}

	Model model;

	for (const auto& shape: shapes) {
		size_t index_offset = 0;

		for (int face_index = 0; face_index < shape.mesh.num_face_vertices.size(); face_index++) {
			unsigned char vertex_count = shape.mesh.num_face_vertices[face_index];
			if (vertex_count != 3) {
				std::cerr << "Tried to load non-triangulated model " << file_path << std::endl;
				return Model::createTriangle();
			}

			int material_id = shape.mesh.material_ids[face_index];
			tinyobj::material_t material = materials[material_id];

			for (size_t v = 0; v < vertex_count; v++) {
				tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

				Vertex new_vertex;

				// vertex position
				new_vertex.position.x = attrib.vertices[3 * idx.vertex_index + 0];
				new_vertex.position.y = attrib.vertices[3 * idx.vertex_index + 1];
				new_vertex.position.z = attrib.vertices[3 * idx.vertex_index + 2];

				// vertex normal
				new_vertex.normal.x = attrib.normals[3 * idx.normal_index + 0];
				new_vertex.normal.y = attrib.normals[3 * idx.normal_index + 1];
				new_vertex.normal.z = attrib.normals[3 * idx.normal_index + 2];

				// vertex uv
				new_vertex.uv.x = attrib.texcoords[2 * idx.texcoord_index + 0];
				// OBJ format has 0 at the bottom of the image, we do the opposite
				new_vertex.uv.y = 1 - attrib.texcoords[2 * idx.texcoord_index + 1];

				// new_vertex.color = new_vertex.normal;
				new_vertex.color = glm::vec3(material.diffuse[0], material.diffuse[1], material.diffuse[2]);

				model.vertices.push_back(new_vertex);
			}

			index_offset += vertex_count;
		}
	}

	// put a small box at the model's "zero point"
	constexpr float size = 0.02f;
	Model hex = Model::createHexahedron(size, size, size);

	for (const auto& vertex : hex.vertices) {
		model.vertices.push_back(vertex);
	}

	return model;
}
