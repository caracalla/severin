#pragma once

#include <model.h>
#include <scene.h>
#include <window_handler.h>


struct Renderer {
	WindowHandler* _window_handler;

	Renderer(WindowHandler* window_handler);

	bool init();

	ModelID uploadModel(const Model model);

	void draw(const Scene* const scene);

	void cleanup();
};
