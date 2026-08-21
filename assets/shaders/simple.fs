#version 330

in vec2 pos;
in vec3 col;
layout (location = 0) out vec4 fragment;
//layout (location = 1) out vec4 color_fragment;

void main() {
  fragment = vec4(col, 1.0);
  //color_fragment = vec4(col, 1.0);
}