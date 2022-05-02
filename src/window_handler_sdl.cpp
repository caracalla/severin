#include <window_handler.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>


struct SDL_Window* sdl_window{ nullptr };


WindowHandler::WindowHandler(int width, int height) :
		_window_width(width),
		_window_height(height) {
	SDL_Init(SDL_INIT_VIDEO);

	SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN);

	sdl_window = SDL_CreateWindow(
		"Vulkan Engine",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		_window_width,
		_window_height,
		window_flags
	);

	SDL_SetRelativeMouseMode(SDL_TRUE);

	_is_running = true;
}

void WindowHandler::cleanup() {
	SDL_DestroyWindow(sdl_window);
}

std::vector<const char*> WindowHandler::getRequiredExtensions() {
	unsigned int count;
	SDL_Vulkan_GetInstanceExtensions(sdl_window, &count, nullptr);

	std::vector<const char*> extensions;
	extensions.resize(count);

	SDL_Vulkan_GetInstanceExtensions(sdl_window, &count, extensions.data());

	return extensions;
}

bool WindowHandler::createSurface(VkInstance instance, VkSurfaceKHR* surface) {
	SDL_bool result = SDL_Vulkan_CreateSurface(sdl_window, instance, surface);

	return result == SDL_TRUE;
}

#ifdef _MSC_VER
#define MOUSE_SENSITIVITY_FACTOR 100
#else
#define MOUSE_SENSITIVITY_FACTOR 1000
#endif

void WindowHandler::handleInput() {
	SDL_Event input_event;

	_mouse_state.reset();

	// Handle events on queue
	while (SDL_PollEvent(&input_event) != 0) {
		// close the window when user alt-f4s or clicks the X button
		if (input_event.type == SDL_QUIT) {
			_is_running = false;
		}

		if (input_event.type == SDL_KEYDOWN) {
			switch (input_event.key.keysym.sym) {
				case SDLK_w:
					_button_states.forward = true;
					break;
				case SDLK_a:
					_button_states.left = true;
					break;
				case SDLK_s:
					_button_states.reverse = true;
					break;
				case SDLK_d:
					_button_states.right = true;
					break;
				case SDLK_q:
					_button_states.rise = true;
					break;
				case SDLK_e:
					_button_states.fall = true;
					break;
				case SDLK_ESCAPE:
					_is_running = false;
					break;
				case SDLK_LSHIFT:
					// reset camera
					_button_states.change_camera = true;
					break;
				default:
					break;
			}
		}

		if (input_event.type == SDL_KEYUP) {
			switch (input_event.key.keysym.sym) {
				case SDLK_w:
					_button_states.forward = false;
					break;
				case SDLK_a:
					_button_states.left = false;
					break;
				case SDLK_s:
					_button_states.reverse = false;
					break;
				case SDLK_d:
					_button_states.right = false;
					break;
				case SDLK_q:
					_button_states.rise = false;
					break;
				case SDLK_e:
					_button_states.fall = false;
					break;
				case SDLK_LSHIFT:
					// reset camera
					_button_states.change_camera = false;
				default:
					break;
			}
		}

		if (input_event.type == SDL_MOUSEBUTTONDOWN) {
			_button_states.action = true;
		}

		if (input_event.type == SDL_MOUSEBUTTONUP) {
			_button_states.action = false;
		}

		if (input_event.type == SDL_MOUSEMOTION) {
			_mouse_state.xOffset = static_cast<float>(input_event.motion.xrel) / MOUSE_SENSITIVITY_FACTOR;
			_mouse_state.yOffset = static_cast<float>(input_event.motion.yrel) / MOUSE_SENSITIVITY_FACTOR;
		}
	}
}

Input::ButtonStates WindowHandler::getButtonStates() {
	return _button_states;
}

Input::MouseState WindowHandler::getMouseState() {
	return _mouse_state;
}