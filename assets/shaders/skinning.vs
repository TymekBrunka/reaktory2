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
uniform mat4 finalBonesMatrices[MAX_BONES];
	
out vec2 TexCoords;
	
void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < 4 ; i++)
    {
        if(boneIds1[i] == -1) 
            continue;
        if(boneIds1[i] >=MAX_BONES) 
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds1[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights1[i];
        //vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
    }
    for(int i = 0 ; i < 4 ; i++)
    {
        if(boneIds2[i] == -1) 
            continue;
        if(boneIds2[i] >=MAX_BONES) 
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        vec4 localPosition = finalBonesMatrices[boneIds2[i]] * vec4(pos,1.0f);
        totalPosition += localPosition * weights2[i];
        //vec3 localNormal = mat3(finalBonesMatrices[boneIds[i]]) * norm;
    }
		
    //mat4 viewModel = view * model;
    mat4 viewModel = view;
    gl_Position =  projection * viewModel * totalPosition;
    TexCoords = tex;
}
