#version 330

in vec2 TexCoords;
out vec4 finalColor;

uniform int diffuse1_idx;
uniform sampler2D diffuse1;
uniform vec4 diffuse_color;

vec4 Blend(in vec4 fragColor, in vec4 textureColor)
{
    vec4 blend;
    //blend = fragColor*(1.0-alpha) + fragColor*textureColor*alpha;
    blend = fragColor*(1.0-textureColor.w) + textureColor;
    return blend;
}

void main()
{
    //finalColor = texture(diffuse1, TexCoords);
    vec4 texcolor = diffuse1_idx != -1 ? texture(diffuse1, TexCoords) : vec4(0,0,0,0);
    finalColor = Blend(diffuse_color, texcolor);
    //finalColor = diffuse_color;
}
