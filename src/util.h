#pragma once

#include <glm/glm.hpp>

#include <chrono>


struct AxisAngle {
	glm::vec3 axis;
	float angle;

	static AxisAngle fromDirection(const glm::vec3& new_direction);
	static AxisAngle fromEulerAngles(const glm::vec3& euler_angles);

	void log();
};


namespace util {
	void log(const char* fmt, ...);
	void logError(const char* fmt, ...);

	void logFrameStats(std::chrono::microseconds last_frame_duration);
	const bool shouldLog();

	void init();

	const float randomFloat(float lower_bound, float upper_bound);
	const int randomInt(int lower_bound, int upper_bound);

	void logVec3(const glm::vec3& vec, const char* fmt = "", ...);
	void logMat4(const glm::mat4& mat, const char* fmt = "", ...);
	void logAxisAngle(const AxisAngle& aa);

	const float getElapsedTime();

	constexpr static float kEpsilon = 0.0001f;

	// by default, everythiing starts off staring down the Z axis in the negative
	// direction (i.e. into the screen, if the player is also facing this direction)
	constexpr glm::vec3 neutral_direction{0.0f, 0.0f, -1.0f};

	const bool areVectorsEqual(const glm::vec3& v1, const glm::vec3& v2);
	const bool isVectorZero(const glm::vec3& vec);
	glm::vec3 safeNormalize(const glm::vec3& vec);
}
