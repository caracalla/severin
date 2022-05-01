#include <engine.h>
#include <renderer.h>
#include <scene.h>
#include <util.h>
#include <window_handler.h>

#include <cstdio>
#include <cstdlib>


int main(int argc, char* argv[]) {
	util::init();

	int window_width = kDefaultWindowWidth;
	int window_height = kDefaultWindowHeight;

	// input arg parsing
	if (argc > 1) {
		if (argc == 3) {
			window_width = atoi(argv[1]);
			window_height = atoi(argv[2]);
		} else {
			printf("usage: severin [window_width window_height]\n");
			return EXIT_SUCCESS;
		}
	}

	// setup
	WindowHandler window_handler(window_width, window_height);
	Renderer renderer(&window_handler);

	if (!renderer.init()) {
		util::logError("renderer failed to init");
		return EXIT_FAILURE;
	}

	float aspect_ratio = static_cast<float>(window_width) / window_height;
	Camera camera(aspect_ratio);
	Scene scene(camera);
	Engine engine(&window_handler, &scene, &renderer);

	// load level
	std::string level_file = "assets/basic.level";

	if (!engine.loadLevelFile(level_file)) {
		util::logError("failed to load level %s", level_file.c_str());
		return EXIT_FAILURE;
	}

	// let's go!
	engine.run();

	util::log("all done");

	return EXIT_SUCCESS;
}
