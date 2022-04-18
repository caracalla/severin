#pragma once

#include <scene.h>
#include <window_handler.h>


struct Renderer {
	WindowHandler* _window_handler;

	Renderer(WindowHandler* window_handler) : _window_handler(window_handler) {}

	bool init();

	uint16_t uploadOBJModel(
			const std::string& assets_basedir,
			const std::string& file_name);

	void draw(const Scene* const scene);

	void cleanup();
};
