#include <util.h>

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

void updateFrameStats(std::chrono::microseconds last_frame_duration) {
	time_since_last_second += last_frame_duration;

	if (last_frame_duration < min_frame_duration) {
		min_frame_duration = last_frame_duration;
	}

	if (last_frame_duration > max_frame_duration) {
		max_frame_duration = last_frame_duration;
	}

	frame_count += 1;

	should_log = time_since_last_second > std::chrono::seconds(1);
}

void util::logFrameStats(std::chrono::microseconds last_frame_duration) {
	updateFrameStats(last_frame_duration);

	if (should_log) {
		double average_frame_time = time_since_last_second.count() / frame_count;

		log(
				"FPS: %d, avg duration: %.0f ms, min duration: %d, max duration: %d",
				frame_count,
				average_frame_time,
				min_frame_duration.count(),
				max_frame_duration.count());

		// log("FPS: %d", frame_count);

		frame_count = 0;
		time_since_last_second = std::chrono::microseconds(0);
		min_frame_duration = std::chrono::microseconds::max();
		max_frame_duration = std::chrono::microseconds(0);
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