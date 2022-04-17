#include <renderer.h>
#include <scene.h>
#include <util.h>
#include <window_handler.h>


struct Engine {
  WindowHandler* _window_handler;
  Scene* _scene;
  Renderer* _renderer;

  Engine(WindowHandler* window_handler, Scene* scene, Renderer* renderer) :
			_window_handler(window_handler),
			_scene(scene),
			_renderer(renderer) {};

  bool isRunning() {
    return _window_handler->isRunning();
  }

  void run() {
		using namespace std::chrono;

		steady_clock::time_point frame_start =
				steady_clock::now();

		while (isRunning()) {
			// draw current scene
			_renderer->draw(_scene);
			
			// get frame duration
			steady_clock::time_point frame_end = steady_clock::now();
			microseconds frame_duration = duration_cast<microseconds>(frame_end - frame_start);
			frame_start = frame_end;

			util::logFrameStats(frame_duration);

			// get inputs
			_window_handler->handleInput();
			Input::ButtonStates button_states = _window_handler->getButtonStates();
			Input::MouseState mouse_state = _window_handler->getMouseState();

			// handle movement and stuff
			_scene->step(frame_duration, button_states, mouse_state);
		}

		_renderer->cleanup();
  }
};
