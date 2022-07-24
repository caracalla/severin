#pragma once

#include <model.h>
#include <scene.h>
#include <window_handler.h>


struct Renderer {
	WindowHandler* _window_handler;

	Renderer(WindowHandler* window_handler);

	const bool init() const;

	const ModelID uploadModel(const Model model) const;

	void draw(const Scene* const scene) const;

	void cleanup() const;
};
