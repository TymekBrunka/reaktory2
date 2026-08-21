#version 330

in vec2 position;
in vec3 color;
out vec2 pos;
out vec3 col;

void main() {
  gl_Position = vec4(position, 0.0, 1.0);
  pos = position;
  col = color;
}