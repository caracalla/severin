#include <util.h>

#include <glm/gtc/epsilon.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <cstdarg>
#include <fstream>
#include <iostream>
#include <random>


void util::log(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("INFO: ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}

void util::logError(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("ERROR: ");
	vprintf(fmt, args);
	printf("\n");
	va_end(args);
}


// frame duration logging
int frame_count = 0;
std::chrono::microseconds time_since_last_second = std::chrono::microseconds(0);
std::chrono::microseconds min_frame_duration = std::chrono::microseconds::max();
std::chrono::microseconds max_frame_duration = std::chrono::microseconds(0);

bool should_log = true;

// now that we're forcing 60 fps in Engine::run(), the extra frame stats no longer make
// sense (e.g. avg duration is always ~16000us)
#define LOG_EXTRA_FRAME_STATS 0

void updateFrameStats(std::chrono::microseconds last_frame_duration) {
#if LOG_EXTRA_FRAME_STATS
	if (last_frame_duration < min_frame_duration) {
		min_frame_duration = last_frame_duration;
	}

	if (last_frame_duration > max_frame_duration) {
		max_frame_duration = last_frame_duration;
	}
#endif // LOG_EXTRA_FRAME_STATS

	time_since_last_second += last_frame_duration;

	frame_count += 1;

	should_log = time_since_last_second > std::chrono::seconds(1);
}

void util::logFrameStats(std::chrono::microseconds last_frame_duration) {
	updateFrameStats(last_frame_duration);

	if (should_log) {
#if LOG_EXTRA_FRAME_STATS
		double average_frame_time = time_since_last_second.count() / frame_count;

		log(
				"FPS: %d, avg duration: %.0f us, min duration: %d, max duration: %d",
				frame_count,
				average_frame_time,
				min_frame_duration.count(),
				max_frame_duration.count());

		min_frame_duration = std::chrono::microseconds::max();
		max_frame_duration = std::chrono::microseconds(0);
#else
		log("FPS: %d", frame_count);
#endif // LOG_EXTRA_FRAME_STATS

		frame_count = 0;
		time_since_last_second = std::chrono::microseconds(0);
	}
}

const bool util::shouldLog() {
	return should_log;
}


// random helpers
std::mt19937 mt;

void util::init() {
	std::random_device rd;
	mt = std::mt19937(rd());
}

const float util::randomFloat(float lower_bound, float upper_bound) {
	std::uniform_real_distribution<float> dist(lower_bound, upper_bound);

	return dist(mt);
}

const int util::randomInt(int lower_bound, int upper_bound) {
	return (int)randomFloat(lower_bound, upper_bound);
}

void util::logVec3(const glm::vec3& vec, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("INFO: ");
	vprintf(fmt, args);
	printf(": ");
	va_end(args);

	printf("%f, %f, %f\n", vec.x, vec.y, vec.z);
}

void util::logMat4(const glm::mat4& mat, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	printf("INFO: ");
	vprintf(fmt, args);
	printf(":\n");
	va_end(args);

	printf("    %f, %f, %f, %f\n", mat[0].x, mat[0].y, mat[0].y, mat[0].w);
	printf("    %f, %f, %f, %f\n", mat[1].x, mat[1].y, mat[1].y, mat[1].w);
	printf("    %f, %f, %f, %f\n", mat[2].x, mat[2].y, mat[2].y, mat[2].w);
	printf("    %f, %f, %f, %f\n", mat[3].x, mat[3].y, mat[3].y, mat[3].w);
}

void util::logAxisAngle(const AxisAngle& aa) {
	logVec3(aa.axis, "angle: %f, axis: ", aa.angle);
}

const float util::getElapsedTime() {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	return std::chrono::duration<float, std::chrono::seconds::period>(
			currentTime - startTime).count();
}

const bool util::areVectorsEqual(const glm::vec3& v1, const glm::vec3& v2) {
	return glm::all(glm::epsilonEqual(v1, v2, kEpsilon));
}

const bool util::isVectorZero(const glm::vec3& vec) {
	return areVectorsEqual(glm::vec3(0.0f), vec);
}

glm::vec3 util::safeNormalize(const glm::vec3& vec) {
	return isVectorZero(vec) ? vec : glm::normalize(vec);
}

AxisAngle AxisAngle::fromDirection(const glm::vec3& new_direction) {
	AxisAngle result;
	result.axis = util::safeNormalize(glm::cross(util::neutral_direction, new_direction));
	result.angle = acos(glm::dot(util::neutral_direction, new_direction));
	return result;
}

AxisAngle AxisAngle::fromEulerAngles(const glm::vec3& euler_angles) {
	glm::mat4 rotation_matrix = glm::rotate(
			glm::rotate(
					glm::rotate(
							glm::mat4(1.0f),
							euler_angles.y,
							glm::vec3(0.0f, 1.0f, 0.0f)),
					euler_angles.x,
					glm::vec3(1.0f, 0.0f, 0.0f)),
			euler_angles.z,
			glm::vec3(0.0f, 0.0f, 1.0f));

	glm::vec3 new_direction = glm::vec3(rotation_matrix * glm::vec4(util::neutral_direction, 0.0f));
	return AxisAngle::fromDirection(new_direction);
}

void AxisAngle::log() {
	util::logAxisAngle(*this);
}
