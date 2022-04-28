#version 450

layout (location = 0) in vec3 inColor;
layout (location = 1) in vec3 texCoord;

layout (location = 0) out vec4 outFragColor;

// layout (set = 0, binding = 1) uniform SceneBuffer {
//   vec4 fogColor; // w is for exponent (?)
//   vec4 fogDistances; // x for min, y for max
//   vec4 ambientColor;
//   vec4 sunlightDirection; // w for sun power
//   vec4 sunlightColor;
// } scene;

void main() {
  outFragColor = vec4(inColor, 1.0f);
}
