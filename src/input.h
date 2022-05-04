#pragma once

namespace Input {
	struct ButtonStates {
		bool forward = false;
		bool reverse = false;
		bool left = false;
		bool right = false;
		bool rise = false;
		bool fall = false;
		bool jump = false;
		bool sprint = false;
		bool action = false;
		bool change_camera = false;
	};

	struct MouseState {
		float xOffset = 0;
		float yOffset = 0;

		void reset() {
			// prevent drift from old inputs sticking around
			xOffset = 0;
			yOffset = 0;
		}
	};
}
