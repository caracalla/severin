#include <engine.h>
#include <renderer.h>
#include <scene.h>
#include <util.h>
#include <window_handler.h>

#include <cstdio>
#include <cstdlib>


int main(int argc, char* argv[]) {
	util::init();

	int width = kDefaultWindowWidth;
	int height = kDefaultWindowHeight;

	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}

	WindowHandler window_handler(width, height);
	Renderer renderer(&window_handler);

	if (!renderer.init()) {
		util::logError("renderer failed to init");
		return EXIT_FAILURE;
	}

	Scene scene{};
	Engine engine(&window_handler, &scene, &renderer);

	std::string level_file = "test.level";

	if (!engine.loadLevelFile(level_file)) {
		util::logError("failed to load level %s", level_file.c_str());
		return EXIT_FAILURE;
	}

	engine.run();

	util::log("all done");

	return EXIT_SUCCESS;
}
