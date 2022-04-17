#include <renderer.h>
#include <util.h>


bool Renderer::init() {
  util::log("vulkan");
  return true;
}

void Renderer::draw(Scene* scene) {
  // draw
}

void Renderer::cleanup() {}
