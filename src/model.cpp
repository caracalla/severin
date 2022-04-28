#include <model.h>

#include <tiny_obj_loader.h>

#include <iostream>


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

Model Model::createHexahedron(glm::vec3 box_min, glm::vec3 box_max) {
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
	glm::vec3 color = glm::vec3(0.8f, 0.0f, 0.8f);

	auto insertVertex = [&model, &color](glm::vec3 position) {
		Vertex vertex;
		vertex.color = color;
		vertex.position = position;
		model.vertices.push_back(vertex);
	};

	// build top
	insertVertex(rtf);
	insertVertex(rtr);
	insertVertex(ltr);

	insertVertex(ltr);
	insertVertex(ltf);
	insertVertex(rtf);

	color = glm::vec3(0.2f, 0.0f, 0.2f);

	// build bottom
	insertVertex(rbf);
	insertVertex(lbf);
	insertVertex(lbr);

	insertVertex(lbr);
	insertVertex(rbr);
	insertVertex(rbf);

	// build front
	insertVertex(rtf);
	insertVertex(ltf);
	insertVertex(lbf);

	insertVertex(lbf);
	insertVertex(rbf);
	insertVertex(rtf);

	color = glm::vec3(0.4f, 0.0f, 0.4f);

	// build rear
	insertVertex(rtr);
	insertVertex(rbr);
	insertVertex(lbr);

	insertVertex(lbr);
	insertVertex(ltr);
	insertVertex(rtr);

	color = glm::vec3(0.6f, 0.0f, 0.6f);

	// build left
	insertVertex(ltf);
	insertVertex(ltr);
	insertVertex(lbr);

	insertVertex(lbr);
	insertVertex(lbf);
	insertVertex(ltf);

	// build right
	insertVertex(rtf);
	insertVertex(rbf);
	insertVertex(rbr);

	insertVertex(rbr);
	insertVertex(rtr);
	insertVertex(rtf);

	return model;
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

		return Model::createHexahedron(glm::vec3(-1, -1, -1), glm::vec3(1, 1, 1));
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
	constexpr float min = -0.01;
	constexpr float max = 0.01;
	Model hex = Model::createHexahedron(
			glm::vec3(min, min, min),
			glm::vec3(max, max, max));

	for (const auto& vertex : hex.vertices) {
		model.vertices.push_back(vertex);
	}

	return model;
}