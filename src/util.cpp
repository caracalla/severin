#include <util.h>

#include <glm/gtc/epsilon.hpp>

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
				"FPS: %d, avg duration: %.0f ms, min duration: %d, max duration: %d",
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

bool util::shouldLog() {
	return should_log;
}


// random helpers
std::mt19937 mt;

void util::init() {
	std::random_device rd;
	mt = std::mt19937(rd());
}

float util::randomFloat(float lower_bound, float upper_bound) {
	std::uniform_real_distribution<float> dist(lower_bound, upper_bound);

	return dist(mt);
}

int util::randomInt(int lower_bound, int upper_bound) {
	return (int)randomFloat(lower_bound, upper_bound);
}

void util::logVec3(glm::vec3 vec, const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	printf(": ");
	va_end(args);
	printf("%f, %f, %f\n", vec.x, vec.y, vec.z);
}

void util::logMat4(glm::mat4 mat) {
	printf("    %f, %f, %f, %f\n", mat[0].x, mat[0].y, mat[0].y, mat[0].w);
	printf("    %f, %f, %f, %f\n", mat[1].x, mat[1].y, mat[1].y, mat[1].w);
	printf("    %f, %f, %f, %f\n", mat[2].x, mat[2].y, mat[2].y, mat[2].w);
	printf("    %f, %f, %f, %f\n\n", mat[3].x, mat[3].y, mat[3].y, mat[3].w);
}

float util::getElapsedTime() {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	return std::chrono::duration<float, std::chrono::seconds::period>(
			currentTime - startTime).count();
}

bool util::isVectorZero(glm::vec3 vec) {
	return glm::all(glm::epsilonEqual(vec, glm::vec3(0.0f), util::kEpsilon));
}
