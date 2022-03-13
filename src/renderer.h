#pragma once

#include <scene.h>
#include <window_handler.h>


struct Renderer {
	WindowHandler* _window_handler;

	Renderer(WindowHandler* window_handler) : _window_handler(window_handler) {}

	bool init();

	void draw(Scene* scene);

	void cleanup();
};
