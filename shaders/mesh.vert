#version 460

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;
layout (location = 2) in vec3 vertexColor;
layout (location = 3) in vec3 vertexUV;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec3 texCoord;

layout (set = 0, binding = 0) uniform CameraBuffer {
  // mat4 view;
  // mat4 projection;
  mat4 viewXprojection;
} camera;

struct Object {
  mat4 modelMatrix;
};

// std140 enforces how the memory is laid out and aligned
layout (std140, set = 1, binding = 0) readonly buffer ObjectBuffer {
  Object objects[];
} objectData;

// using ObjectBuffer SSBO instead of push constants
// layout (push_constant) uniform constants {
//   vec4 data;
//   mat4 render_matrix;
// } PushConstants;

layout (push_constant) uniform constants {
  float time;
} PushConstants;

mat4 rotate(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0f - c;

  return mat4(
    oc * axis.x * axis.x + c,          oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0f,
    oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z - axis.x * s, 0.0f,
    c * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z * s,          oc * axis.z * axis.z + c,          0.0f,
    0.0f,                              0.0f,                              0.0f,                              1.0f
  );
}

void main() {
  mat4 modelMatrix = objectData.objects[gl_BaseInstance].modelMatrix;
  mat4 transformMatrix = camera.viewXprojection * modelMatrix;

  gl_Position = transformMatrix * vec4(vertexPosition, 1.0f);

  float s = sin(PushConstants.time);
  float c = cos(PushConstants.time);
  vec3 light_dir = vec3(s, 1.0f, c);
  vec3 normal = inverse(transpose(mat3(modelMatrix))) * normalize(vertexNormal);

  float min_light = 0.2f;
  float light_intensity = max(dot(light_dir, normal), min_light);

  // normal = inverse(transpose(mat3(model))) * normalize(aNormal);

  outColor = vertexColor * light_intensity;
  // outColor = vertexColor;
  texCoord = vertexUV;
}
