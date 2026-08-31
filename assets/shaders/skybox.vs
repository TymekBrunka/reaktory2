#version 330

in vec3 pos;
in vec2 uv;
//in vec3 normal;
out vec2 out_uv;

uniform mat4 projection;
uniform mat4 view;

void main() {
    //gl_Position = skybox_camera * vec4(pos, 1.0);
    gl_Position = (projection * mat4(mat3(view))) * vec4(pos, 1.0);
    out_uv = uv;
}