#pragma once

#include <glm/glm.hpp>

#include <chrono>


namespace util {
	void log(const char* fmt, ...);
	void logError(const char* fmt, ...);

	void logFrameStats(std::chrono::microseconds last_frame_duration);
	bool shouldLog();

	void init();

	float randomFloat(float lower_bound, float upper_bound);
	int randomInt(int lower_bound, int upper_bound);

	void logVec3(glm::vec3 vec, const char* fmt = "", ...);
	void logMat4(glm::mat4 mat, const char* fmt = "", ...);
	float getElapsedTime();

	constexpr static float kEpsilon = 0.0001f;

	bool areVectorsEqual(glm::vec3 v1, glm::vec3 v2);
	bool isVectorZero(glm::vec3 vec);
	glm::vec3 safeNormalize(glm::vec3 vec);

	// inputs must be normalized!
	glm::vec3 rotationBetweenTwoVectors(glm::vec3 v1, glm::vec3 v2);
}
