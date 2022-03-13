#include <renderer.h>
#include <util.h>


bool Renderer::init() {
  util::log("vulkan");
  return true;
}

void Renderer::draw(Scene* scene) {
  auto start = std::chrono::steady_clock::now();

  // draw

  auto current_frame_duration_ns = std::chrono::duration_cast<std::chrono::nanoseconds>(
					std::chrono::steady_clock::now() - start);

  util::logFrameStats(current_frame_duration_ns);
}

void Renderer::cleanup() {}
