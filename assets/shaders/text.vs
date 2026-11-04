#version 330
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

#define width 10
#define height 16

in vec2 pos;
in vec2 tex;

void main() {
  gl_Position =  projection * viewModel * vec4(1.0f);
}