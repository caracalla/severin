#include <engine.h>
#include <renderer.h>
#include <scene.h>
#include <util.h>
#include <window_handler.h>

#include <cstdio>
#include <cstdlib>


int main(int argc, char* argv[]) {
	util::init();

	int width = 800;
	int height = 600;

	WindowHandler window_handler(width, height);
	Renderer renderer(&window_handler);

	if (!renderer.init()) {
		util::log("renderer failed to init");
		return EXIT_FAILURE;
	}

	Scene scene{};
	Engine engine(&window_handler, &scene, &renderer);

	engine.run();

	util::log("all done");

	return EXIT_SUCCESS;
}
