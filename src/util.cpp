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
	printf("INFO: ");
	vprintf(fmt, args);
	printf(": ");
	va_end(args);

	printf("%f, %f, %f\n", vec.x, vec.y, vec.z);
}

void util::logMat4(glm::mat4 mat, const char* fmt, ...) {
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

float util::getElapsedTime() {
	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();

	return std::chrono::duration<float, std::chrono::seconds::period>(
			currentTime - startTime).count();
}

bool util::areVectorsEqual(glm::vec3 v1, glm::vec3 v2) {
	return glm::all(glm::epsilonEqual(v1, v2, kEpsilon));
}

bool util::isVectorZero(glm::vec3 vec) {
	return areVectorsEqual(glm::vec3(0.0f), vec);
}

glm::vec3 util::safeNormalize(glm::vec3 vec) {
	return isVectorZero(vec) ? vec : glm::normalize(vec);
}

#include <glm/gtc/type_ptr.hpp>

glm::vec3 util::rotationBetweenTwoVectors(glm::vec3 v1, glm::vec3 v2) {
	float angle = acos(glm::dot(v1, v2));
	glm::vec3 axis = safeNormalize(glm::cross(v1, v2));

	glm::vec3 result = glm::vec3(0.0f);

	if (abs(angle) > kEpsilon && !isVectorZero(axis)) {
		// glm's version of things (which for some reason doens't work?)
		// glm::mat4 glm_rotation_matrix = glm::rotate(glm::mat4(1.0f), angle, axis);
		// float glm_r11 = glm_rotation_matrix[0][0];
		// float glm_r12 = glm_rotation_matrix[0][1];
		// float glm_r13 = glm_rotation_matrix[0][2];
		// float glm_r21 = glm_rotation_matrix[1][0];
		// float glm_r22 = glm_rotation_matrix[1][1];
		// float glm_r23 = glm_rotation_matrix[1][2];
		// float glm_r31 = glm_rotation_matrix[2][0];
		// float glm_r32 = glm_rotation_matrix[2][1];
		// float glm_r33 = glm_rotation_matrix[2][2];
		// glm::vec3 glm_result = glm::vec3{
		// 	atan2(glm_r32, glm_r33),
		// 	atan2(-1.0f * glm_r31, sqrt(glm_r32 * glm_r32 + glm_r33 * glm_r33)),
		// 	atan2(glm_r21, glm_r11)
		// };

		// my interpretation
		float c = cos(angle);
		float s = sin(angle);
		float t = 1 - c;
		glm::mat4 my_rotation_matrix(0.0f);
		float my_r11 = axis.x * axis.x * t + c;
		float my_r12 = axis.x * axis.y * t - axis.z * s;
		float my_r13 = axis.x * axis.z * t + axis.y * s;
		float my_r21 = axis.y * axis.x * t + axis.z * s;
		float my_r22 = axis.y * axis.y * t + c;
		float my_r23 = axis.y * axis.z * t - axis.x * s;
		float my_r31 = axis.z * axis.x * t - axis.y * s;
		float my_r32 = axis.z * axis.y * t + axis.x * s;
		float my_r33 = axis.z * axis.z * t + c;
		glm::vec3 my_result = glm::vec3{
			atan2(my_r32, my_r33),
			atan2(-1.0f * my_r31, sqrt(my_r32 * my_r32 + my_r33 * my_r33)),
			atan2(my_r21, my_r11)
		};

		result = my_result;
	}

	return result;
}
