#version 330

layout (location = 0) out vec4 fragment;
//layout (location = 1) out vec4 color_fragment;

void main() {
  fragment = vec4(0.1953125, 0.484375, 0.6328125, 0.1953125);
  //color_fragment = vec4(col, 1.0);
}