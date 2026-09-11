#version 330

in vec2 TexCoords;
out vec4 finalColor;

uniform sampler2D diffuse1;
uniform vec4 diffuse_color;

vec4 Blend(in vec4 fragColor, in vec4 textureColor, in float alpha)
{
    vec4 blend;
    blend = fragColor*(1.0-alpha) + fragColor*textureColor*alpha;
    return blend;
}

void main()
{
    //finalColor = texture(texture0, TexCoords);
    vec4 texcolor = texture(diffuse1, TexCoords);
    finalColor = Blend(diffuse_color, texcolor, texcolor.a);
    //finalColor = vec4(0,0,0,1);
}
