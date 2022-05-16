#include <engine.h>
#include <renderer.h>
#include <scene.h>
#include <util.h>
#include <window_handler.h>

#include <cstdio>
#include <cstdlib>


void printUsage() {
	printf("usage: severin [-w window_width] [-h window_height] [-f frames_to_run]\n");
	exit(0);
}

struct ArgumentOptions {
	int window_width = kDefaultWindowWidth;
	int window_height = kDefaultWindowHeight;
	int frames_to_run = 0; // set to non-zero to debug
};

ArgumentOptions parseArguments(int argc, char* argv[]) {
	ArgumentOptions options;

	for (int i = 1; i < argc; i++) {
		std::string arg = std::string(argv[i]);

		if (arg == "-w") {
			i += 1;
			if (i < argc) {
				options.window_width = atoi(argv[i]);
			} else {
				printUsage();
			}
		} else if (arg == "-h") {
			i += 1;
			if (i < argc) {
				options.window_height = atoi(argv[i]);
			} else {
				printUsage();
			}
		} else if (arg == "-f") {
			i += 1;
			if (i < argc) {
				options.frames_to_run = atoi(argv[i]);
				util::log("only running for %d frames", options.frames_to_run);
			} else {
				printUsage();
			}
		} else {
			printUsage();
		}
	}

	return options;
}


int main(int argc, char* argv[]) {
	util::init();

	ArgumentOptions options = parseArguments(argc, argv);

	// setup
	WindowHandler window_handler(options.window_width, options.window_height);
	Renderer renderer(&window_handler);

	if (!renderer.init()) {
		util::logError("renderer failed to init");
		return EXIT_FAILURE;
	}

	float aspect_ratio =
			static_cast<float>(options.window_width) / options.window_height;
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
	engine.run(options.frames_to_run);

	util::log("all done");

	return EXIT_SUCCESS;
}
