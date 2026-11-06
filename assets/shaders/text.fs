#version 330

uniform sampler2D text;
uniform vec3 ID;

//in vec3 col;
in vec2 texPos;
layout (location = 0) out vec4 fragment;
layout (location = 1) out vec4 colorBuffer;

void main() {
  fragment = texture(text, texPos);
  //fragment = vec4(0.0, 0.0, 1.0, 1.0);
  colorBuffer = vec4(ID, 1.0);
}