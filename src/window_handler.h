#pragma once

#include <input.h>

#include <vulkan/vulkan.h>

#include <vector>


constexpr int kDefaultWindowWidth = 800;
constexpr int kDefaultWindowHeight = 600;


// wraps SDL or GLFW to handle window, surface, and input
struct WindowHandler {
	int _window_width = kDefaultWindowWidth;
	int _window_height = kDefaultWindowHeight;

	bool _is_running = false;

	Input::ButtonStates _button_states;
	Input::MouseState _mouse_state;

	WindowHandler(int width, int height);
	void cleanup();

	const bool isRunning() const {
		return _is_running;
	}

	const std::vector<const char*> getRequiredExtensions() const;

	const bool createSurface(VkInstance instance, VkSurfaceKHR* surface) const;

	void handleInput();
	const Input::ButtonStates getButtonStates() const;
	const Input::MouseState getMouseState() const;
};
