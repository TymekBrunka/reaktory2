#version 330 core

in vec2 out_uv;
layout (location = 0) out vec4 frag_color;

uniform sampler2D skybox;

void main() {
    frag_color = texture(skybox, out_uv);
}