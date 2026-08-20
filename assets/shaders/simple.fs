#version 330

in vec2 pos;
in vec3 col;
out vec4 fragment;

void main() {
  fragment = vec4(col, 1.0);
}