#pragma once

#include <glm/glm.hpp>

#include <chrono>


namespace util {
	void log(const char* fmt, ...);
	void logError(const char* fmt, ...);

	void logFrameStats(std::chrono::nanoseconds last_frame_duration_ns);
	bool shouldLog();

	void init();

	float randomFloat(float lower_bound, float upper_bound);
	int randomInt(int lower_bound, int upper_bound);

	void logMat4(glm::mat4 mat);
	float getElapsedTime();
}
