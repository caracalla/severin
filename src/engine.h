#pragma once

#include <renderer.h>
#include <scene.h>
#include <util.h>
#include <window_handler.h>

#include <string>


struct Engine {
	WindowHandler* _window_handler;
	Scene* _scene;
	Renderer* _renderer;

	Engine(WindowHandler* window_handler, Scene* scene, Renderer* renderer) :
			_window_handler(window_handler),
			_scene(scene),
			_renderer(renderer) {};

	const ModelID uploadModel(Model model) const {
		return _renderer->uploadModel(model);
	}

	const bool loadLevelFile(const std::string& filename) const;

	bool isRunning() {
		return _window_handler->isRunning();
	}

	void run(const int frames_to_run);
};
