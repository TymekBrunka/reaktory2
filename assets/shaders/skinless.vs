#version 330

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;

layout(location = 3) in ivec4 boneIds1; 
layout(location = 4) in ivec4 boneIds2; 
layout(location = 5) in vec4 weights1;
layout(location = 6) in vec4 weights2;
	
uniform mat4 projection;
uniform mat4 view;
//uniform mat4 model;
	
const int MAX_BONES = 100;
const int MAX_BONE_INFLUENCE = 8;
	
out vec2 TexCoords;
	
void main()
{
    gl_Position =  projection * view * vec4(pos, 1);
    TexCoords = tex;
}
