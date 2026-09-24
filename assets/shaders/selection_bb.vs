#version 330

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform vec3 min;
uniform vec3 max;

in vec3 position;

void main() {
  vec3 pos = vec3(
    position.x < 0 ? min.x : max.x,
    position.y < 0 ? min.y : max.y,
    position.z < 0 ? min.z : max.z
  );
  gl_Position = projection * view * model * vec4(pos, 1.0);
}