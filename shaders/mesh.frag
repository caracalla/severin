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

layout (push_constant) uniform constants {
  float time_elapsed;
  float resolution_x;
  float resolution_y;
} PushConstants;

#define time PushConstants.time_elapsed
#define fragCoord_norm gl_FragCoord.xy / vec2(PushConstants.resolution_x, PushConstants.resolution_y)

#define PI 3.14159265

float random (vec2 st) {
  return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

void main() {
  vec3 color = inColor;

  outFragColor = vec4(color, 1.0f);
}
